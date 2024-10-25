import os
import subprocess

def dir_create_then_enter(path):
  if os.path.isdir(path):
    return False
  os.mkdir(dir_name)
  os.chdir(dir_name)
  return True

def git_clone(repo_url, branch_or_tag_name=None):
  args = ['git', 'clone', repo_url]
  if branch_or_tag_name is not None:
    args.extend(['-b', branch_or_tag_name])
  subprocess.run(args)
