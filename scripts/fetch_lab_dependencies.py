import os
import subprocess

def git_clone(repo_url, branch_or_tag_name=None):
  args = ['git', 'clone', repo_url]
  if branch_or_tag_name is not None:
    args.extend(['-b', branch_or_tag_name])
  subprocess.run(args)

if __name__ == '__main__':
  try:
    os.mkdir('libs/')
  finally:
    os.chdir('libs/')
    git_clone('https://dawn.googlesource.com/dawn', 'chromium/6670')
    git_clone('https://github.com/g-truc/glm', '1.0.1')
    git_clone('https://github.com/syoyo/tinygltf', 'v2.9.3')
