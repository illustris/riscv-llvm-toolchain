#! /usr/bin/env python2
import re
import sys

def usage():
    print "%s <filter-file>" % sys.argv[0]

if len(sys.argv) != 2:
    usage()
    sys.exit(1)

def read_ignore_list(ignore_list_path):
   with open(ignore_list_path, "r") as ignore_list_f:
     ignore_list = ignore_list_f.readlines()

   ignore_list = map(lambda l: l.strip(), ignore_list)
   # Filter out comment line and empty line
   ignore_list = filter(lambda l: not l.startswith("#") \
                                  and len(l) != 0, ignore_list)
   return ignore_list

def count_case(prefix, data):
   f = filter(lambda s:s.startswith(prefix), data)
   total_count = len(f)
   x = set(map(lambda d: d.split(" ")[1] , f))
   case_count = len(x)
   return total_count, case_count

def find_begin_line(idx, log_data):
  idx = idx - 1
  while idx != 0:
    s = log_data[idx]
    if len(s) > 0 and s[0].isupper():
      return idx
    idx = idx - 1
  raise Exception("find_begin_line fail")

def read(sum_path, log_path):
  with open(sum_path, "r") as sum_file:
    sum_data = sum_file.readlines()

  with open(log_path, "r") as log_file:
    log_data = log_file.readlines()

  def filter_func(s):
    return s.startswith("FAIL:") \
           or s.startswith("XPASS:") \
           or s.startswith("UNRESOLVED:")

  # First, filter out only FAIL, XPASS and UNRESOLVED
  sum_data = filter(filter_func, sum_data)
  # And then filter out all "test for warnings"
  sum_data = filter(lambda s:"(test for warnings" not in s \
                             and "FAIL: compiler driver" not in s \
                             and "(test for errors, line" not in s \
                             and "(test for bogus messages, line" not in s,sum_data)
  # Filter out all "scan-tree-dump", "scan-rtl-dump" and "scan-ipa-dump-times"
  sum_data = filter(lambda s:" scan-tree-dump" not in s \
                             and " scan-rtl-dump" not in s \
                             and " scan-ipa-dump" not in s,sum_data)


  ignore_list = read_ignore_list(sys.argv[1])
  def filter_ignore(s):
    for ignore in ignore_list:
      if ignore in s:
        return False
    return True

  # Last, filter out all contain in ignore_list.
  sum_data = filter(filter_ignore, sum_data)

  # Create a filtered log
  find_count = 0
  not_found_count = 0
  start_index = 0
  log_data_len = len(log_data)
  with open(log_path + ".filtered", "w") as f:
    for result in sum_data:
      # Find the result in where
      idx = start_index
      while idx < log_data_len:
        log = log_data[idx]
        if log == result:
          find_count = find_count + 1
          start_index = idx
          begin_line = find_begin_line(idx, log_data)
          f.write("".join(log_data[begin_line+1:idx+1]))
          break
        idx = idx + 1
      else:
        raise Exception("Not found match entry `%s` in log(%s not match %s?)" \
                         % (result, log_path, sum_path))

  fail_count, fail_case_count = count_case("FAIL:", sum_data)
  xpass_count, xpass_case_count = count_case("XPASS:", sum_data)
  unresolve_count, unresolve_case_count = count_case("UNRESOLVED:", sum_data)
  with open(sum_path + ".filtered", "w") as f:
    f.write("".join(sum_data))
  print "".join(sum_data)
  print "%-28s %6d (%6d case)" % ("# of unexpected failures", fail_count, fail_case_count)
  print "%-28s %6d (%6d case)" % ("# of unexpected successes", xpass_count, xpass_case_count)
  print "%-28s %6d (%6d case)" % ("# of unresolved testcases", unresolve_count, unresolve_case_count)

read("gcc.sum", "gcc.log")
