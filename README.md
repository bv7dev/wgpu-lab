# wgpu-lab

### Todos

- [ ] fix bug: multi-threaded to_device seems to sometimes cause crashes
- [ ] use limits to find min/max values for number of buffers, etc.
- [ ] tutorial: index buffer to be able to load models later
- [ ] tutorial: loading file chaper important for learning to load images and gamma correction
- [ ] tutorial: shader uniforms
- [x] fix bug: test_buffers crashes in the end (bad function call)
- [x] fix bug: program crashes on minimize (tested with sample_pipeline, maybe all others affected, too) 
- [x] fix bug: error callback not working
- [x] think about: maybe get rid of use of unordered map for windows, because it is iterated in main loop, 
      which would be faster with vector. performance test showed that umap is as fast as vector iteration 
      with small number of elements


### Ideas

##### doc generator
- script that can parse all source files to compile a documentation in obsidian vault format
- includes evaluated to connect nodes in obsidian graph view
- comments parsed to describe structs and functions

##### fuzz testing
- generate randomized tests for creating, deleting and using various lab objects in random orders
  to discover undefined behaviors and interface weaknesses
