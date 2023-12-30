KICAD


schema editor -> generate bom  (use standaard bom2csv -> creates xml file)


use of python2.7: to install modules: C:\Python27\python -m pip install <module>

In the bom directory:

parts.json  ; selector + component data..


C:\Python27\python mkassemblybom.py

creates 

bompre.dat
bom.dat
bompost.dat


https://www.sharelatex.com  make an account


upload bom.tex and the dat files -> compile -> nice bom pdf.


prices:

read prices from octopart:

C:\Python27\python updateprices.py


generateds dat files..
C:\Python27\python mkstandardbom.py

upload bom.tex and the dat files -> compile -> nice price bom pdf.


