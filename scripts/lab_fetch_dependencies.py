import os
import sys

from lab_helpers import dir_create_then_enter, git_clone

try:
  assert len(sys.argv) == 3, 'Not enough arguments'
  assert sys.argv[1] == '--root', 'Missing "--root" argument'

  os.chdir(sys.argv[2]) # change into wgpu-lab's root directory

  if dir_create_then_enter('libs'):
    print("fetching wgpu-labs's dependencies... \n")

    git_clone('https://dawn.googlesource.com/dawn', 'chromium/6670')
    git_clone('https://github.com/g-truc/glm', '1.0.1')
    git_clone('https://github.com/syoyo/tinygltf', 'v2.9.3')

  else:
    print("already got wgpu-lab's dependencies")
    print("if broken, delete libs directory to reset")

except AssertionError as err:
  print('Error:', err)
  print('Usage: "python scripts/lab_fetch_dependencies.py --root [LAB_ROOT_DIR]"')
