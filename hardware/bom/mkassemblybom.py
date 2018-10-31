

import BOM

optionset = set(['NOSWCLK','REGPA','PA','LVDS25','ETH25','ETH','VERSA','VBIAS','VOP',
	'RXLPF','RXNPOL','TXLPF','TXPREAMP','TXSW','TXLP','LED','FRONTIO','ADC','THERMAL',
	'PATR','EXTPTT','EXTFILTER','VERSAOSC','CASE','PROGRAMMER'])

## 'NOASSEMBLY' to include parts needed by not put on by assembly house

bom = BOM.BOM("../radioberry.xml",optionset=optionset)

pre = """\\section*{Radioberry 2.0 beta3 BOM}
Assembly Build - \\today"""

bom.LaTeXAssemblyPrint(pre,['Mouser','Digi-Key'])


