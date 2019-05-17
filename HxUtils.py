''' HxUtils.py 2018 by x01 '''

import os, sys, re

def convert_endlines(filepath, tounix=True):
	try:
		oldfile = open(filepath, 'rb+')
		path, name = os.path.split(filepath)
		newfile = open(path + '$' + name, 'ba+')

		old = b'\r'
		new = b''
		if not tounix:
			old = b'\n'
			new = b'\r\n'

		while True:
			olddata = oldfile.read(1024)
			newdata = olddata.replace(old, new)
			newfile.write(newdata)
			if len(olddata) < 1024:
				break
			
		newfile.close()
		oldfile.close()

		os.remove(filepath)
		os.rename(path + '$' + name, filepath)

	except IOError as e:
		print(e)

def convert_endlines_for_dir(dirpath='.', tounix=True, extname='.py'):
	'''
	批量转换换行符
	'''
	if len(sys.argv) == 2:
		dirpath = sys.argv[1]
	elif len(sys.argv) == 3:
		dirpath, tounix = (sys.argv[1], sys.argv[2])
	elif len(sys.argv) == 4:
		dirpath, tounix, extname = (sys.argv[1], sys.argv[2], sys.argv[3])

	for (thisdir, subshere, fileshere) in os.walk(dirpath):
		for filename in fileshere:
			if filename.endswith(extname):
				convert_endlines(os.path.join(thisdir,filename), tounix)


def changeCase(dir, isupper):
	"""
	Change file and directory case by isupper parameter.
	"""
	files = os.listdir(dir)
	for f in files:
		path = os.path.join(dir,f)
		newpath = ""
		if (isupper):
			newpath = path.upper()
		else:
			newpath = path.lower()
		if os.path.isdir(path):
			os.rename(path, newpath)
			changeCase(path, isupper)
			continue
		os.rename(path, newpath)

def changeExtName(dir, orgExtName, newExtName):
	"""
	Change file org-ext-name  to new-ext-name.
	Example: changeExtName('.', '.css', '.less')
	"""
	filelist=os.listdir(dir)   
	for files in filelist: 
		path=os.path.join(dir,files) 
		if os.path.isdir(path):   
			changeExtName(path, orgExtName, newExtName)
			continue
		filename=os.path.splitext(files)[0]
		filetype=os.path.splitext(files)[1] 
		if (filetype != orgExtName):
			continue
		newPath=os.path.join(dir, filename + newExtName); 
		os.rename(path,newPath) 

def del_files(filename, currdir='.'):
	""" 递归删除 currdir 目录下的所有 filename """
	dir = os.path.abspath(currdir)
	files = os.listdir(dir) 
	for f in files:
		path = os.path.join(dir, f)
		if os.path.isdir(path):
			del_files(filename, path)
			dir = os.path.dirname(path)
			continue
		if os.path.basename(path) == filename:
			os.chdir(dir)
			print(path)
			cmd = "rm -f " + filename  
			os.system(cmd)


def convert_2to3():
	''' usage: python3 HxUtils.py -w [dirpath] '''
	import sys
	from lib2to3.main import main

	sys.exit(main("lib2to3.fixes"))


if __name__ == '__main__':
	#convert_endlines_for_dir()
	#convert_2to3()
	
	# path = '/home/x01/lab/py/demo/res'
	#changeExtName(path, '.less', 'less')
	#changeCase(path, False)

	dir = 'oranges'
	del_files('c.img', dir)


	
