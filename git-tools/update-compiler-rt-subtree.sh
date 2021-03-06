#!/bin/bash 
#
# Update Scout's llvm subtree with the main llvm trunk.  Note that
# llvm's primary version control is handled with subversion but is
# mirrored with a git snapshot...
#

. scout-local/utils/update-subtree-funcs.sh 

# Make sure we're not on the master branch -- at "home" we lock this
# down so people can't muck with the master branch (as getting there
# means you pass compile and regression checks).  However, we'll leave
# this as a sanity check...
branch=$(active_git_branch)
if [ $branch == 'master' ]; then
    echo "This script must not be run with the 'master' branch checked out..."
    exit 1;
fi

# Check for 'dirty' status... 
dirty=$(is_git_dirty)
if [ "$dirty" == "yes" ]; then 
    echo "Your repository has uncommitted changes.  Aborting..."
    exit 1;
fi 


if [ ! -d llvm/projects/compiler-rt ]; then 
    echo "Unable to find llvm/projects/compiler-rt in current working directory."
    exit 1;
fi 

git subtree pull --squash -P llvm/projects/compiler-rt http://llvm.org/git/compiler-rt.git master 

