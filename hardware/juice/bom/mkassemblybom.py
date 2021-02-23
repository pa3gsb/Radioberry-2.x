

import BOM

optionset = set([])

## 'NOASSEMBLY' to include parts needed by not put on by assembly house

bom = BOM.BOM("../radioberry-juice.xml",optionset=optionset)

pre = """\\section*{Radioberry-juice beta1 BOM}
Assembly Build - \\today"""

bom.LaTeXAssemblyPrint(pre,['Mouser','Digi-Key'])


