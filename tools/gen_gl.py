import re
import subprocess
import sys
import time
import os
import json
import argparse
import concurrent
from pathlib import Path
from threading import Lock
import xml.etree.ElementTree as ET

locallySupported = [
    'GL_COMPRESSED_RGBA_BPTC_UNORM',
    'GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM',
    'GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT',
    'GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT',
    'GL_COMPRESSED_RGB_S3TC_DXT1_EXT',
    'GL_COMPRESSED_RGBA_S3TC_DXT1_EXT',
    'GL_COMPRESSED_RGBA_S3TC_DXT3_EXT',
    'GL_COMPRESSED_RGBA_S3TC_DXT5_EXT',
    'GL_COMPRESSED_RGB8_ETC2',
    'GL_COMPRESSED_SRGB8_ETC2',
    'GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2',
    'GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2',
    'GL_COMPRESSED_RGBA8_ETC2_EAC',
    'GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC',
    'GL_COMPRESSED_R11_EAC',
    'GL_COMPRESSED_SIGNED_R11_EAC',
    'GL_COMPRESSED_RG11_EAC',
    'GL_COMPRESSED_SIGNED_RG11_EAC',
]

untypedFormats = {
    'GL_RED': True,
    'GL_RG': True,
    'GL_RGB': True,
    'GL_RGBA': True,
    'GL_DEPTH_COMPONENT': True,
    'GL_COMPRESSED_RED': True,
    'GL_COMPRESSED_RG': True,
    'GL_COMPRESSED_RGB': True,
    'GL_COMPRESSED_RGBA': True,
}

class Parser:
    def __init__(self, args):
        self.args = args
        wd = os.getcwd()
        glXmlFile = wd + '/external/gl/xml/gl.xml'
        tree = ET.parse(glXmlFile)
        self.root = tree.getroot()
        self.deprecated = {}
        self.enums = {}
        self.enumsByValue = {}
        self.internalFormats = []

    def matchAttribs(self, node, match, inverseMatch = None):
        if inverseMatch is not None:
            if self.matchAttribs(node, inverseMatch):
                return False
        for key in match:
            if not key in node.attrib:
                return False
            if match[key] is not None and match[key] != node.attrib[key]:
                return False       
        return True

    def findMatchingEnums(self, match, inverseMatch = None, regex = None):
        result = {}
        for enums in self.root.iter('enums'):
            if not self.matchAttribs(enums, match, inverseMatch): 
                continue
            for enum in enums.iter('enum'):
                name = enum.attrib['name']
                if (regex is not None) and (regex.match(name) is None):
                    #print("Ignoring regex match failure " + name)
                    continue

                value = int(enum.attrib['value'], 16)
                if name in self.deprecated: 
                    #print("Ignoring deprecated enum " + name)
                    continue
                result[name] = value
        return result
    
    def registerMatchingEnums(self, match, inverseMatch = None, regex = None):
        enums = self.findMatchingEnums(match, inverseMatch=inverseMatch, regex=regex)
        result = []
        for name in enums:
            value = enums[name]
            if value in self.enumsByValue: 
                #print("Ignoring deprecated duplicate value " + name + " previously registered as " + self.enumsByValue[value])
                continue
            self.enumsByValue[value] = name
            self.enums[name] = value
            result.append(name)
        return result



    def parse(self):
        for remove in self.root.iter('remove'):
            for enum in remove.iter('enum'):
                name = enum.attrib['name']
                self.deprecated[name] = True

        self.registerMatchingEnums({'vendor': 'ARB'}, {'group': None})

        # Add the core internal format
        for groups in self.root.iter('groups'):
            for group in groups.iter('group'):
                if group.attrib['name'] != 'InternalFormat': continue
                for enum in group.iter('enum'):
                    name = enum.attrib['name']
                    if not name in self.enums: continue
                    if name in untypedFormats: continue
                    self.internalFormats.append(name)

        # Add the ASTC, ETC and DXT formats.  I wish there were a better way to find these
        # heuristically.  But there's no indication in an extension XML definition 
        # of what 'group' a given enum belongs to

        # ASTC textures
        textures = self.registerMatchingEnums({'vendor':'OES', 'start':'0x93B0'})
        self.internalFormats.extend(textures)
        # ETC textures
        textures = self.registerMatchingEnums({'vendor':'OES', 'start':'0x9270'})
        self.internalFormats.extend(textures)

        dxtRegex = re.compile(r'^GL_COMPRESSED_.*(DXT\d(?!_ANGLE)|BPTC)')
        # RGB DXT textures
        textures = self.registerMatchingEnums({'vendor':'INTEL', 'start':'0x83F0'}, regex=dxtRegex)
        self.internalFormats.extend(textures)
        # SRGB DXT textures
        textures = self.registerMatchingEnums({'vendor':'NV', 'start':'0x8C10'}, regex=dxtRegex)
        self.internalFormats.extend(textures)
        # BPTC textures
        textures = self.registerMatchingEnums({'vendor':'NV', 'start':'0x8E10'}, regex=dxtRegex)
        self.internalFormats.extend(textures)

        print("Done")

def main():
    parser = Parser({})
    parser.parse()
    foundFormats = {}
    for format in parser.internalFormats:
        foundFormats[format] = True
    for format in locallySupported:
        if not format in foundFormats:
            print("Failed to locate " + format)

    for format in parser.internalFormats:
        
        print("    " + format[3:] + " = " + ("0x%x" % parser.enums[format]) + ",")
    print("done")



    # glEnums = {}
    # glEnumsByValue = {}
    # # Core OpenGL enums
    # glInternalFormats = []




    # print(glInternalFormats)
            



# parser = ArgumentParser(description='Generate shader artifacts.')
# parser.add_argument('--commands', type=argparse.FileType('r'), help='list of commands to execute')
# parser.add_argument('--spirv-binaries', type=str, help='location of the SPIRV binaries')
# parser.add_argument('--build-dir', type=str, help='The build directory base path')
# parser.add_argument('--source-dir', type=str, help='The root directory of the git repository')
# parser.add_argument('--scribe', type=str, help='The scribe executable path')
# parser.add_argument('--debug', action='store_true')
# parser.add_argument('--force', action='store_true', help='Ignore timestamps and force regeneration of all files')
# parser.add_argument('--dry-run', action='store_true', help='Report the files that would be process, but do not output')

# args = None
# if len(sys.argv) == 1:
#     # for debugging
#     spirvPath = os.environ['VULKAN_SDK'] + '/bin'
#     #spirvPath = expanduser('~//VulkanSDK/1.1.82.1/x86_64/bin')
#     sourceDir = expanduser('~/git/hifi')
#     buildPath = sourceDir + '/build_noui'
#     scribePath = buildPath + '/tools/scribe/Release/scribe'
#     commandsPath = buildPath + '/libraries/shaders/shadergen.txt'
#     shaderDir = buildPath + '/libraries/shaders'
#     testArgs = '--commands {} --spirv-binaries {} --scribe {} --build-dir {} --source-dir {}'.format(
#         commandsPath, spirvPath, scribePath, shaderDir, sourceDir
#     ).split()
#     #testArgs.append('--debug')
#     #testArgs.append('--force')
#     #testArgs.append('--dry-run')
#     args = parser.parse_args(testArgs)
# else:
#     args = parser.parse_args()

main()
