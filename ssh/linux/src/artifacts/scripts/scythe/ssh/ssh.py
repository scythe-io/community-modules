# Copyright (c) SCYTHE, Inc. Use is subject to agreement.

import argparse
import shlex
import struct


# noinspection PyUnusedLocal
def create_parser(db, os="windows"):
    class ArgumentParser(argparse.ArgumentParser):
        def error(self, message):
            raise ValueError(message)

    epilog =  '  scythe.ssh --message "Hello, world!"\n'
    parser = ArgumentParser(prog="ssh", description="Echo the request back as response",
                            epilog=epilog)
    parser.add_argument("--message", help="Message to echo back", required=True)
    return parser


def usage(db, os):
    """Return the usage of this module as a string

    :return str: Usage string for this module
    """
    return create_parser(db, os).format_help()


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
        raise ValueError("Error: --message argument is missing.")
    else:
        argv = shlex.split(command_line, posix=False)

    args = parser.parse_args(argv)        
            
    # We need args.dest and either args.src or args.content
    if not args.message:    
        raise ValueError("Error: --message argument is missing.")
    else:
        args.message = args.message.strip('"')

    return args.message.encode("utf-8")


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
