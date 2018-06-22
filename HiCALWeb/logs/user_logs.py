import click
import json
import re

import sys
from django.core.management.base import OutputWrapper
from django_extensions.management.color import color_style

LOG_RE = "^\\[(?P<timestamp>\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{6})\\] (?P<level>\\w+) \\[(?P<code_ns>[^:]+):(?P<code_line>\\d+)\\] (?P<jmsg>.*)$"
h = re.compile(LOG_RE)

stdout = OutputWrapper(sys.stdout)
stderr = OutputWrapper(sys.stderr)
style = color_style()
stderr.style_func = style.ERROR

@click.command()
@click.argument('input', type=click.Path(exists=True))
@click.argument('user', type=str)
def user_logs(input, user):
    logs = []
    stdout.write(style.SUCCESS("Parsing '{}' logs from '{}' ".format(user, input)))
    with open(input, 'r') as f:
        errors_count = 0
        for l in f:
            matches = h.match(l)
            level = matches.group('level')
            try:
                jmsg = json.loads(matches.group('jmsg'))[0]
                log_user = jmsg['user']
                if log_user != user:
                    continue
                client_time = jmsg['client_time']
                result = jmsg['result']
                logs.append((client_time, level, user, result))
            except:
                errors_count += 1
                stdout.write(style.ERROR("{}: Could not parse log message: {}".format(
                    errors_count, matches.group('jmsg'))))

    out = "{}.{}".format(user, input)
    with open(out, 'wt') as f:
        for l in logs:
            line = "{} {} {} : {}\n".format(l[0], l[1], l[2], l[3])
            f.write(line)

    stdout.write(style.SUCCESS("Parsing {} from {} completed. "
                               "Output written to {}".format(user, input, out)))

if __name__ == '__main__':
    user_logs()
