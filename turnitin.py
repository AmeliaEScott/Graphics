#!/usr/bin/env python3

import os
import shutil
import glob
import zipfile

extras = ["cow.obj", "teapot.obj", "teddy.obj"]

folders = glob.glob("sources*")
last_assignment = 0
for folder in folders:
    try:
        last_assignment = max(last_assignment, int(folder[-1]))
    except:
        pass

print("Last assignment: {}. Current assignment: {}".format(last_assignment, last_assignment + 1))
newfolder = "scott_t_a{}".format(last_assignment + 1)
print("Created new folder {}".format(newfolder))
try:
    shutil.rmtree(newfolder)
    print("Removed existing {}".format(newfolder))
except:
    pass

try:
    os.remove("/Users/Timmy/Desktop/" + newfolder + ".zip")
    print("Removed {}.zip from the Desktop.".format(newfolder))
except:
    pass
os.mkdir(newfolder)


shutil.copytree("libs", newfolder + "/libs")
shutil.copy("CMakeLists.txt", newfolder + "/CMakeLists.txt")
shutil.copytree("sources", newfolder + "/sources")
for extra in extras:
    print("Added file {} to zip.".format(extra))
    shutil.copy(extra, newfolder + "/" + extra)

with zipfile.ZipFile("/Users/Timmy/Desktop/" + newfolder + ".zip", "w", compression=zipfile.ZIP_DEFLATED) as ziph:
    for root, dirs, files in os.walk(newfolder):
        print("Adding folder {} to zip.".format(root))
        for file in files:
            ziph.write(os.path.join(root, file))
shutil.rmtree(newfolder)
print("Done! Created file {}.zip on Desktop.".format(newfolder))
