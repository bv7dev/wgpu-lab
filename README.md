# wgpu-lab

### Todos

- [ ] fix bug: program crashes on minimize (tested with sample_pipeline, maybe all others affected, too) 
- [ ] fix bug: error callback not working
- [ ] use limits to find min/max values for number of buffers, etc.
- [ ] fix bug: test_buffers crashes in the end (bad function call)
- [ ] think about: maybe get rid of use of unordered map for windows, because it is iterated in main loop, which would be
      faster with vector


### Ideas

##### doc generator
- script that can parse all source files to compile a documentation in obsidian vault format
- includes evaluated to connect nodes in obsidian graph view
- comments parsed to describe structs and functions

##### fuzz testing
- generate randomized tests for creating, deleting and using various lab objects in random orders
  to discover undefined behaviors and interface weaknesses
