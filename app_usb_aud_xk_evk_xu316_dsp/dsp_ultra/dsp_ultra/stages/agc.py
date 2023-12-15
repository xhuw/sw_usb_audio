from ..design.stage import Stage

AGC_CONFIG = """
---
module:
  agc:
    gain:
      type: float
includes:
  - stdint.h
  - "dspt_module.h"
"""

class AGC(Stage):
    def __init__(self, **kwargs):
        super().__init__(config=AGC_CONFIG, **kwargs)
        self.create_outputs(self.n_in)


    def process(self, in_channels):
        """
        Run AGC on the input channels and return the output

        Args:
            in_channels: list of numpy arrays

        Returns:
            list of numpy arrays.
        """
        ret = []
        for channel in in_channels:
            ret.append(channel * self["gain"])
        return ret

