import os
import sys

def loadbinaryfile(filename):
    f = open(filename, 'rb')
    try:
        return f.read()
    finally:
        f.close()

def savebinaryfile(filename, data):
    f = open(filename, 'wb')
    try:
        f.write(data)
    finally:
        f.close()

def makewrapper(targetdir, template, target, cmd=''):
    target = os.path.join(targetdir, target)
    targetdir = os.path.dirname(target)
    if not os.path.isdir(targetdir):
        os.makedirs(targetdir)
    savebinaryfile(target, loadbinaryfile(template) + cmd)

makewrapper(*sys.argv[1:])
