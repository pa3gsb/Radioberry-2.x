

import BOM

optionset = set([])

## 'NOASSEMBLY' to include parts needed by not put on by assembly house

bom = BOM.BOM("../radioberry-preamp.xml",optionset=optionset)

pre = """\\section*{Radioberry-preamp beta1 BOM}
Assembly Build - \\today"""

bom.LaTeXAssemblyPrint(pre,['Mouser','Digi-Key'])


