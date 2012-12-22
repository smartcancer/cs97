import datetime

def unix_time(dt,start):
    epoch = datetime.datetime.utcfromtimestamp(0)
    delta = dt - start
    return delta.total_seconds()

def unix_time_millis(dt,start):
    return unix_time(dt,start) * 1000.0

def main():
  start = datetime.datetime.now()
  while(1):
    print unix_time_millis(datetime.datetime.now(),start)

main()
