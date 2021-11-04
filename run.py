import glob, os
#os.chdir("/")

# change project variable to match your sketch name
project = 'esp32-firmware-update'
binFiles = glob.glob(project + "*.bin")
	
#read the arduino sketch
file = open(project + '.ino', 'r')
lines = file.readlines()
file.close()

versionCode = 0
baseUrl = ""
checkFile = ""
fileName = ""

for line in lines:
	if "int currentVersion" in line:
		versionCode = int(''.join(i for i in line if i.isdigit()))
		fileName = "firmware_v"+str(versionCode)+".bin"
	if "String baseUrl" in line:
		baseUrl = line.split('"')[1::2][0]
	if "String checkFile" in line:
		checkFile = line.split('"')[1::2][0]

#generate the json file
updater = open(checkFile, 'w')
updater.write("{\n\t\"versionCode\" : "+str(versionCode)+",\n\t\"fileName\" : \""+fileName+"\"\n}")
updater.close()

#rename the binary file
if len(binFiles) > 0 and binFiles[0] != fileName:
	os.replace(binFiles[0], fileName)
