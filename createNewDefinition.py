import samweb_client as swc

# Settings

defName = "mydataset_maxeventfile"

# Open file
fIn = "files_sorted_maxeventfile.list"

with open(fIn,'rb') as fileIn:
  fileList = fileIn.read()

# Create definition
samweb = swc.SAMWebClient(experiment='uboone')

samweb.createDefinition(defName, fileList, user="myusername", group="uboone", description="1 files per run ")


print "Created definition: %s" %(defName)

print "done!"
