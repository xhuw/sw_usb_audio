
from typing import Iterable
from pathlib import Path
from .graph import Graph
from .stage import StageOutput
from .thread import Thread
import graphviz
from IPython import display
import yaml
import subprocess


class Pipeline:
    """
    Top level class which is a container for a list of threads that
    are connected in series.
    """

    def __init__(self, n_in):
        self.graph = Graph()
        self._threads = []

        self.i = [StageOutput() for _ in range(n_in)]
        for i, input in enumerate(self.i):
            self.graph.add_edge(input)
            input.source_index = i
        self.o = None

    def add_thread(self):
        ret = Thread(id=len(self._threads), graph=self.graph)
        self._threads.append(ret)
        return ret

    def set_outputs(self, output_edges):
        """set the pipeline outputs, configures the output channel index"""
        for i, edge in enumerate(output_edges):
            edge.dest_index = i

    def validate(self):
        """pipeline must be straight with no branches"""
        graphdict = {}
        for edge in self.graph.edges:
            try:
                graphdict[edge.dest].append(edge.source)
            except KeyError:
                graphdict[edge.dest] = [edge.source]
        for dests in graphdict.values():
            for a, b in zip(dests[:-1], dests[1:]):
                if a is not b:
                    raise ValueError("pipeline must be linear")
        
    def draw(self):
        """Render a dot diagram of this pipeline"""
        dot = graphviz.Digraph()
        dot.clear()
        for i_thread, thread in enumerate(self._threads):
            with dot.subgraph(name=f"cluster_{i_thread}") as subg:
                subg.attr(label=f"thread {i_thread}")
                for i, n in enumerate(self.graph.nodes):
                    if thread.contains_stage(n):
                        subg.node(n.id.hex, f"{i}: {type(n).__name__}")
        for e in self.graph.edges:
            source = e.source.id.hex if e.source is not None else "start"
            dest = e.dest.id.hex if e.dest is not None else "end"
            dot.edge(source, dest)
        display.display_svg(dot)

    def resolve_pipeline(self):
        """
        Generate a dictionary with all of the information about the thread.
        Actual stage instances not included.
        """
        # 1. Order the graph
        sorted_nodes = self.graph.sort()
        
        # 2. assign nodes to threads
        threads = [[] for _ in range(len(self._threads))]
        for i, thread in enumerate(self._threads):
            for node in sorted_nodes:
                if thread.contains_stage(node):
                    threads[i].append([node.index, node.name])


        edges = []
        for edge in self.graph.edges:
            source = [edge.source.index, edge.source_index] if edge.source is not None else [None, edge.source_index]
            dest = [edge.dest.index, edge.dest_index] if edge.dest is not None else [None, edge.dest_index]
            edges.append([source, dest])

        node_configs = {node.index: node.get_config() for node in self.graph.nodes}

        module_definitions = {node.name: node.yaml_dict for node in self.graph.nodes}

        return {"threads": threads, "edges": edges, "configs": node_configs, "modules": module_definitions}
        
def send_config_to_device(pipeline: Pipeline, host_app = "xvf_host", protocol="usb"):
    """Send all non-default config values to the device"""
    config = pipeline.resolve_pipeline()["configs"]
    for instance, instance_config in config.items():
        for command, value in instance_config.items():
            if isinstance(value, list) or isinstance(value, tuple):
                value = " ".join(str(v) for v in value)
            else:
                value = str(value)
            subprocess.run([host_app, "--use", protocol, "--instance-id", str(instance), 
                            command, *value.split()])

def filter_edges_by_thread(resolved_pipeline):
    """get thread input edges, output edges and internal edges for all threads"""
    dest_in_thread = lambda edge, thread: edge[1][0] in (t[0] for t in thread)
    source_in_thread = lambda edge, thread: edge[0][0] in (t[0] for t in thread)
    ret = []

    for thread in resolved_pipeline["threads"]:
        input_edges = {}
        output_edges = {}
        interal_edges = []
        for edge in resolved_pipeline["edges"]:
            sit = source_in_thread(edge, thread)
            dit = dest_in_thread(edge, thread)
            if sit and dit:
                interal_edges.append(edge)
            elif sit:
                if edge[1][0] is None:
                    # pipeline output
                    di = "pipeline_out"
                else:
                    for di, dthread in enumerate(resolved_pipeline["threads"]):
                        if dest_in_thread(edge, dthread):
                            break
                try:
                    output_edges[di].append(edge)
                except KeyError:
                    output_edges[di] = [edge]
            elif dit:
                if edge[0][0] is None:
                    # pipeline input
                    si = "pipeline_in"
                else:
                    for si, sthread in enumerate(resolved_pipeline["threads"]):
                        if source_in_thread(edge, sthread):
                            break
                try:
                    input_edges[si].append(edge)
                except KeyError:
                    input_edges[si] = [edge]
        ret.append((input_edges, interal_edges, output_edges))
    return ret


def generate_dsp_threads(resolved_pipeline, block_size = 1):
    """
    void dsp_thread(chanend_t* input_c, chanend_t* output_c, module_states, module_configs) {

        int32_t edge0[BLOCK_SIZE];
        int32_t edge1[BLOCK_SIZE];
        int32_t edge2[BLOCK_SIZE];
        int32_t edge3[BLOCK_SIZE];

        for(;;) {
            // input from 2 source threads
            int read_count = 2;
            while(read_count) {
                select {
                    input_c[0]: chan_in_buf(edge0); read_count--;
                    input_c[1]: chan_in_buf(edge1); read_count--;
                }
            }

            modules[0]->process_sample(
                (int32_t*[]){edge0, edge1}, 
                (int32_t*[]){edge2, edge3}, 
                modules[0]->state, 
                &modules[0]->control
            );

            chan_out_buf(output_c[0], edge2);
            chan_out_buf(output_c[1], edge3);
        }
    }
    """
    all_thread_edges = filter_edges_by_thread(resolved_pipeline)
    file_str = ""
    for thread_index, (thread_edges, thread) in enumerate(zip(all_thread_edges, resolved_pipeline["threads"])):
        func = f"DECLARE_JOB(dsp_thread{thread_index}, (chanend_t*, chanend_t*, module_instance_t**));\n"
        func += f"void dsp_thread{thread_index}(chanend_t* c_source, chanend_t* c_dest, module_instance_t** modules) {{\n"
        
        in_edges, internal_edges, all_output_edges = thread_edges
        all_edges = []
        for temp_in_e in in_edges.values():
            all_edges.extend(temp_in_e)
        all_edges.extend(internal_edges)
        for temp_out_e in all_output_edges.values():
            all_edges.extend(temp_out_e)
        for i in range(len(all_edges)):
            func += f"\tint32_t edge{i}[{block_size}];\n"
        func += "\twhile(1) {\n"
        func += f"\tint read_count = {len(in_edges)};\n"
        func += "\tSELECT_RES(\n"
        first = True
        for i, _ in enumerate(in_edges.values()):
            if not first:
                func += ",\n"
            first = False
            func += f"\t\tCASE_THEN(c_source[{i}], case_{i})"
        func += "\n\t) {\n"
        
        for i, edges in enumerate(in_edges.values()):
            func += f"\t\tcase_{i}: {{\n"
            for edge in edges:
                func += f"\t\t\tchan_in_buf_word(c_source[{i}], (void*)edge{all_edges.index(edge)}, {block_size});\n"
            func += f"\t\t\tif(!--read_count) break;\n\t\t\telse continue;\n\t\t}}\n"
        func += "\t}\n"

        for stage_thread_index, stage in enumerate(thread):
            # thread stages are already ordered during pipeline resolution
            input_edges = [edge for edge in all_edges if edge[1][0] == stage[0]]
            input_edges.sort(key = lambda e: e[1][1])
            input_edges = ", ".join(f"edge{all_edges.index(e)}" for e in input_edges)
            output_edges = [edge for edge in all_edges if edge[0][0] == stage[0]]
            output_edges.sort(key = lambda e: e[0][1])
            output_edges = ", ".join(f"edge{all_edges.index(e)}" for e in output_edges)

            func += f"\tmodules[{stage_thread_index}]->process_sample(\n"
            func += f"\t\t(int32_t*[]){{{input_edges}}},\n"
            func += f"\t\t(int32_t*[]){{{output_edges}}},\n"
            func += f"\t\tmodules[{stage_thread_index}]->state, &modules[{stage_thread_index}]->control);\n"

        for out_index, edges in enumerate(all_output_edges.values()):
            for edge in edges:
                func += f"\tchan_out_buf_word(c_dest[{out_index}], (void*)edge{all_edges.index(edge)}, {block_size});\n"

        func += "\t}\n}\n"
        file_str += func
    return file_str


def generate_dsp_main(pipeline: Pipeline, out_dir = "build/dsp_pipeline"):
    """
    Generate the sourcecode for dsp_main 
    
    TODO -  needs to support parallel threads i.e. each
    thread can talk to more than one other thread.
    """
    out_dir = Path(out_dir)
    out_dir.mkdir(exist_ok=True)

    resolved_pipe = pipeline.resolve_pipeline()
    threads = resolved_pipe["threads"]

    n_threads = len(threads)

    chans = ["chan_in", *(f"chan_{i}" for i in range(n_threads))]
    
    

    dsp_main = """
#include "dspt_main.h"
#include <xcore/select.h>
#include <xcore/channel.h>
"""
    dsp_main += generate_dsp_threads(resolved_pipe)
    dsp_main += """
#pragma stackfunction 1000
void dspt_xcore_main(chanend_t c_data, chanend_t c_control)
{
"""
    for chan in chans:
        dsp_main += f"channel_t {chan} = chan_alloc();\n"

    total_modules = 0
    for i, thread in enumerate(threads):
        n_stages = len(thread)
        total_modules += n_stages
        dsp_main += f"const int32_t num_modules_thread{i} = {n_stages};\n"

    dsp_main += f"int total_num_modules = {total_modules};\n"
    dsp_main += f"module_instance_t* all_modules[{total_modules}];\n\n"

    for i, thread in enumerate(threads):
        dsp_main += f"module_instance_t* modules{i}[] = {{\n"
        for mod_i, mod_name in thread:
            dsp_main += f"\t{mod_name}_init({mod_i}),\n"
        dsp_main += f"}};\n\n"
        for this_i, (mod_i, mod_name) in enumerate(thread):
            dsp_main += f"all_modules[{mod_i}] = modules{i}[{this_i}];\n"


    dsp_main += f"""
     PAR_JOBS(
        PJOB(dsp_data_transport_thread, (c_data, {chans[0]}.end_a, {chans[-1]}.end_b)),
        PJOB(dsp_control_thread, (c_control, all_modules, total_num_modules))"""

    for i, (chan_a, chan_b) in enumerate(zip(chans[:-1], chans[1:])):
        dsp_main += f",\n        PJOB(dsp_thread, ({chan_a}.end_b, {chan_b}.end_a, modules{i}, num_modules_thread{i}))"

    dsp_main += "\n    );\n}\n"

    (out_dir / "dsp_main.c").write_text(dsp_main)

    yaml_dir = out_dir/"yaml"
    yaml_dir.mkdir(exist_ok=True)

    for name, defintion in resolved_pipe["modules"].items():
        (yaml_dir / f"{name}.yaml").write_text(yaml.dump(defintion))
