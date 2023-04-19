import sys
import os
import samweb_cli

try:
  samweb = samweb_cli.SAMWebClient(experiment="uboone")
except:
  raise Exception('Not able to open up the SAMWeb Connection')


input_file_list = open(sys.argv[1], 'r')
output_file_list = open(sys.argv[2], 'w')

numFiles = 0
countRuns = 0

countSubRuns = 0

test = 0;

output_file_list.write("file_name ")

for line in input_file_list:
  numFiles += 1
  
  line = line.strip()
  
  metadata = samweb.getMetadata(line)
  if 'event_count' not in metadata.keys():
    print "cannot find event in this file, skip"
    continue
  #print metadata
  event_count = metadata['event_count']
  print line
  print "event_count: ", event_count
  #print type(event_count)

  keySet = line.split("-")
  current_run = keySet[2][2:]

  if numFiles == 1:
    previous_run = current_run
    max_event_count = event_count
    max_event_line = line
    countSubRuns += 1

  if numFiles > 1:
    if (current_run != previous_run):
      countRuns += 1
      previous_run = current_run
      print "max_event_count in previous_run: ", max_event_count
      print "max_event_line: ", max_event_line
      output_file_list.write(max_event_line + ", ")
      max_event_count = event_count
      max_event_line = line
      test += countSubRuns
      print "countSubRuns: ", countSubRuns
      countSubRuns = 1
    else:
      countSubRuns += 1
      if event_count > max_event_count:
        max_event_count = event_count
        max_event_line = line

  #if numFiles > 3:
  #  break
  #  exit(1)

print "max_event_count: ", max_event_count
print "max_event_line: ", max_event_line
output_file_list.write(max_event_line)
output_file_list.close()

countRuns += 1
print "countSubRuns: ", countSubRuns
test += countSubRuns

print "numFiles: ", numFiles
print "countRuns: ", countRuns
print "test: ", test

print "Created file list: %s" %(output_file_list)
print "----- Done! --------"
