from lab_helpers import *

if dir_create_then_enter('libs'):
  print("fetching wgpu-labs's dependencies... \n")

  git_clone('https://dawn.googlesource.com/dawn', 'chromium/6670')
  git_clone('https://github.com/g-truc/glm', '1.0.1')
  git_clone('https://github.com/syoyo/tinygltf', 'v2.9.3')

else:
  print("wgpu-lab's dependencies already fetched")
  print("delete libs directory to reset")
