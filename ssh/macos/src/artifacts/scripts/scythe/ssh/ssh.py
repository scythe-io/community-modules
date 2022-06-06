# Copyright (c) SCYTHE, Inc. Use is subject to agreement.

import argparse
import shlex
import struct
import json
from os.path import basename

__fs = None 

def has_vfs_path(source):
    return source.lower().startswith("vfs:/")

def read_vfs_file(path):
    global __fs
    if not __fs:
        from vfs import ScytheVFS
        __fs = ScytheVFS()
    return __fs.read_file(path)


# noinspection PyUnusedLocal
def create_parser(db, os="windows"):
    class ArgumentParser(argparse.ArgumentParser):
        def error(self, message):
            raise ValueError(message)

    epilog =  '  scythe.ssh --username admin --password letmein --host 192.168.2.1 --command whoami\n'
    parser = ArgumentParser(prog="ssh", description="Send commands to a remote host, transfer files.",
                            epilog=epilog)
    parser.add_argument("--host", help="A single host to login to", required=True, type=str)
    parser.add_argument("--username",help="A single username to login as",required=True, type=str)
    parser.add_argument("--password",help="The ssh password to login as",required=True,type=str)
    parser.add_argument("--port",help="Port that SSH Server is listening on: Default (22)",required=False, default="22")
    parser.add_argument("--command",help="Command or list of commands to run on a target. Example: ls -lah; whoami; ps -aux;",required=False,nargs="*",type=str,default=[])
    parser.add_argument("--filename", help="File to transfer to remote host via SCP. Example: VFS:/users/LOCAL/user1/implant.out", required=False)
    parser.add_argument("--dirpath",help="Directory Path on remote host to write to. Example: /home/user1/. Please make sure it is writeable by ssh user", required=False)
    return parser


def usage(db, os):
    """Return the usage of this module as a string

    :return str: Usage string for this module
    """
    return create_parser(db, os).format_help()

# noinspection PyUnusedLocal
def succeeded(db, request,response):
    result = False
    if response and len(response) > 72:
        content = response[72:].tobytes().decode('utf-8')
        if not "Error: " in content:
            result = True
    return result

# noinspection PyUnusedLocal
def tags(reserved, request, response):
    """
    :param reserved: Reserved for future use
    :param request: Original request sent to device
    :param response: Reply from device for request
    :return: return a list of strings
    :rtype: list
    """
    r = []
    if len(request) > 0:
        r = ["scythe", "att&ck", "att&ck-technique:T1563","att&ck-technique:T1563.001","att&ck-technique:T1021","att&ck-technique:T1021.004","att&ck-technique:T1544"]
    return r


# noinspection PyUnusedLocal
def create_message_body(db, command_line, campaign_name, endpoint_name):
    """Create a SCYTHE message body

    :param db: used only to retrieve operating system
    :param str command_line: command line string. If None is provided, command line will be received from sys.argv
    :param campaign_name: ignored
    :param endpoint_name: ignored
    :return str: String with message body
    """
    # You may call: db.get_setting_value("language")
    # This will return a language id string such as: "en-US"
    # You may use this result to present localized strings in the user interface.

    # You may call: db.get_campaign_operating_system_name(campaign_name)
    # This will return "windows" for Windows campaigns.
    parser = create_parser(db, db.get_campaign_operating_system_name(campaign_name))

    if not command_line:
        raise ValueError("Error: argument is missing.")
    else:
        argv = shlex.split(command_line, posix=False)

    args = parser.parse_args(argv)        
            
    # We need args.dest and either args.src or args.content
    if not args.password:    
        raise ValueError("Error: --password argument is missing.")
    if not args.host:
        raise ValueError("Error: --host argument is missing.")
    if not args.username:
        raise ValueError("Error: --user argument is missing.")
    if not args.port:
        raise ValueError("Error: --port argument is missing.")
    else:
        dict_to_send = {
            "username" : args.username,
            "password" : args.password,
            "host" : args.host,
            "port" : args.port,
            "vfs" : "-1",
        }
        if args.command:
            dict_to_send['command'] = ';'.join(args.command) + " ;"
        
        
        if has_vfs_path(args.filename):
            filename = basename(filename)
            file_bytes = read_vfs_file(filename[4:]).decode('utf-8')
            dict_to_send['filename'] = filename
            dict_to_send['file_bytes'] = file_bytes
            dict_to_send['vfs'] = "1"
            if not args.dirpath:
                raise ValueError("Error: --dirpath must be specified")
            dict_to_send["dirpath"] = args.dirpath
        elif args.filename:
            filename = basename(args.filename)
            dict_to_send['filename'] = filename
            dict_to_send['filepath'] = args.filename
            dict_to_send['filebytes'] = "none"
            dict_to_send["vfs"] = "0"
            if not args.dirpath:
                raise ValueError("Error: --dirpath must be specified")
            dict_to_send["dirpath"] = args.dirpath
    
        return json.dumps(dict_to_send).encode("utf-8")    
    


# noinspection PyUnusedLocal
def report(db, request, response, format_):
    """Generate a report for a request and response for this module

    :param db: ignored
    :param request: Request to report on
    :param response: Response to report on
    :param format_: ignored, always pre
    :return tuple(str, str, str): request report, response report, and format
    """
    # size of the response message is response[64:72]
    sz = struct.unpack("<Q", request[64:72].tobytes())[0]
    request_contents = request[72:72 + sz].tobytes().decode("utf-8")

    content = response[72:].tobytes().decode("utf-8")
    return "--message \"%s\"" % request_contents, content, "pre"


def main():
    pass


if __name__ == "__main__":
    main()
