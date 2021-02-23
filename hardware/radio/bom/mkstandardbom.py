

import BOM

optionset = set(['NOSWCLK','REGPA','PA','LVDS25','ETH25','ETH','VERSA','VBIAS','VOP',
	'RXLPF','RXNPOL','TXLPF','TXPREAMP','TXSW','TXLP','LED','FRONTIO','ADC','THERMAL',
	'PATR','EXTPTT','EXTFILTER','VERSAOSC','CASE','PROGRAMMER', 'NOASSEMBLY'])

## 'NOASSEMBLY' to include parts needed but not put on by assembly house

bom = BOM.BOM("../radioberry.xml",optionset=optionset)

pre = """\\section*{Radioberry 2.0beta1 BOM}
Standard Build - \\today"""

bom.LaTeXPrint(pre,['Mouser','Digi-Key'])


