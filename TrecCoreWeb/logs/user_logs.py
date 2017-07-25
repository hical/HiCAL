import click
import json
import re

LOG_RE = "^\\[(?P<timestamp>\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{6})\\] (?P<level>\\w+) \\[(?P<code_ns>[^:]+):(?P<code_line>\\d+)\\] (?P<jmsg>.*)$"
h = re.compile(LOG_RE)

@click.command()
@click.argument('input', type=click.Path(exists=True))
@click.argument('user', type=str)
def user_logs(input, user):
    logs = []
    with open(input, 'r') as f:
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
                logs.append((client_time, level, user, ":", result))
            except Exception:
                print("Could not parse {}".format(jmsg))

    print(logs)

if __name__ == '__main__':
    user_logs()
