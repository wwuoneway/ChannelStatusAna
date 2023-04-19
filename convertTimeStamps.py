import os
import samweb_cli
import datetime
import time

try:
  samweb = samweb_cli.SAMWebClient(experiment="uboone")
except:
  raise Exception('Not able to open up the SAMWeb Connection') 

epoch = datetime.datetime(1970,1,1,0,0,0)
print epoch

# output files from running analyze_channelStatus
directory = "channel_status_files_run1/"

my_file_list = sorted(os.listdir('./'+directory))
for line in list(my_file_list):
  if not line.startswith('run_'):
    my_file_list.remove(line)

old_stamp = 3
for filename in my_file_list:
  f1 = open(directory+filename)
  lines = f1.readlines()
  run = lines[0].strip().split()[2]
  f1.close()

  if len(run) > 5:
    continue
  print "Run "+run
  
  query_string = "run_number="+run+".0 and file_name=PhysicsRun%.ubdaq"
  file_list = samweb.listFiles(query_string)
  #print file_list
  if len(file_list) > 1:
    print "  Error!  File list is bigger than 1!"
    #continue
    exit(1)
  elif not file_list:
    print "Nothing in subrun=0 file list for run "+run+", look for first available subrun"
    query_string = "run_number="+run+" and file_name=%.ubdaq"
    file_list= samweb.listFiles(query_string)
    if not file_list:
      print "Error: no files for run "+run
      exit(1)
  
  metadata = samweb.getMetadata(file_list[0])
  start_time = metadata['start_time'].split('+')[0]

  #print "  "+start_time
  
  stamp = (datetime.datetime.strptime(start_time,"%Y-%m-%dT%H:%M:%S")-epoch).total_seconds()
  if stamp < old_stamp:
    print "Run "+run+" timestamp is: "+str(stamp)+", less than previous: "+str(old_stamp)
    old_stamp = stamp
  else: 
    old_stamp = stamp
 
  f2 = open(directory+filename,'w')
  lines[0] = lines[0].replace(run,str(stamp).split('.')[0])
  f2.write(''.join(lines))
  f2.close()

print "done"
