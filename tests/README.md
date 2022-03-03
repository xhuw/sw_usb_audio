How to run and extend these tests
=================================

To run the tests, **you will need a USB audio regression test rig.** This is a
rig with two stacked multi-channel audio boards. You will need to connect them
to xTags and map the xtags for use with xtagctl. For more info on how to set
this up, see the README in the xtagctl repo.

These tests are written using pytest combined with xtagctl

To build all firmware required to run these tests, use:
```
$ pytest --build-only -n 1
```

To run all the tests without re-building firmware, use:
```
$ pytest --test-only
```

To build everything, then test, run without any arguments:
```
$ pytest -n 1
```

**NOTE:** The `-n 1` prevents xmake from running in parallel. xtagctl is
designed to handle parallel execution, so running multiple executors in
`--test-only` mode is OK.

Adding a test
-------------

To allow each test to build the config it needs, we make heavy use of pytest
fixtures.

To add a board/config to a test, look for the 
``` python
@pytest.mark.parametrize("build", ..., indirect=True)
```
line above the test definition.

For DFU tests, look for:
``` python
@pytest.mark.parametrize("build_with_dfu", ..., indirect=True)
```

For the command line arguments for controlling build/test only to work, any new
test must skip if the `build` or `build_with_dfu` fixture returns `None`:
``` python
if build is None:
    pytest.skip("Build not present")
```