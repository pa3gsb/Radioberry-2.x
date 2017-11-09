
import untangle, json
import pickledb
import urllib
import re
from decimal import Decimal


def LaTeXEscape(s):
    d = {
        '&': r'\&',
        '%': r'\%',
        '$': r'\$',
        '#': r'\#',
        '_': r'\_',
        '{': r'\{',
        '}': r'\}',
        '~': r'\textasciitilde{}',
        '^': r'\^{}',
        '<=': r'$\leq$',
        '>=': r'$\geq$',
        '\\': r'\textbackslash{}',
    }
    regex = re.compile('|'.join(re.escape(unicode(key)) for key in sorted(d.keys(), key = lambda item: - len(item))))
    return regex.sub(lambda match: d[match.group()], s)


class Quote:
    def __init__(self,price,name,url,manufacturer,sku,mpn):
        self.price = price
        self.name = name
        self.url = url
        self.manufacturer = manufacturer.encode('utf-8')
        self.sku = sku.encode('utf-8')
        self.mpn = mpn

    def __cmp__(self,other):
        return cmp( (self.price,self.sku), (other.price,other.sku) )

    def DebugPrint(self):
        print "{0.2f:10} {1:20} {2:30} {3:30} {4}".format(self.price,self.sku,self.name,self.manufacturer,self.url)

    def LaTeXLine(self):
        if self.name == 'Digi-Key':
            sku = '\href{{http://www.digikey.com/product-search/en?keywords={0}}}{{{0}}}'.format(self.sku)
        else:
            sku = self.sku 

        name = self.name.strip('*')

        if sku != '':
            octoparturl = '\href{{http://www.octopart.com/search?q={0}}}{{{0}}}'.format(self.mpn)    
            seller = '\href{{{1}}}{{{0}}}'.format(LaTeXEscape(name),self.url)
        else:
            octoparturl = self.mpn
            seller = name
            sku = '\href{{{1}}}{{{0}}}'.format(LaTeXEscape(name),self.url)

        ##print self.manufacturer,octoparturl,seller,sku,self.price
        ##return "{0} & {1} & {2} & {3} & {4:.2f}".format(self.manufacturer,octoparturl,seller,sku,self.price)
        return "{0} & {1} & {2} & {3:.2f}".format(octoparturl,seller,sku,self.price)


    def WikiLine(self):
        if self.name == 'Digi-Key':
            sku = '[{0}](http://www.digikey.com/product-search/en?keywords={0})'.format(self.sku)
        else:
            sku = self.sku 

        seller = '[{0}]({1})'.format(self.name,self.url)

        octoparturl = '[{0}](http://www.octopart.com/search?q={0})'.format(self.mpn)

        ##print self.manufacturer,octoparturl,seller,sku,self.price
        return "{0} | {1} | {2} | {3} | {4:.2f}".format(self.manufacturer,octoparturl,seller,sku,self.price)



## Special component quotes
special = {
    "HK4100F-DC12V-SHG":[Quote(Decimal(0.50),'*EBay','http://www.ebay.com','','HK4100F-DC12V-SHG','HK4100F-DC12V-SHG')],
    "BN43-202":[Quote(Decimal(0.50),'*Kits&Parts','http://www.kitsandparts.com','','BN43-202','BN43-202')],
    "BN43-2402":[Quote(Decimal(0.25),'*Kits&Parts','http://www.kitsandparts.com','','BN43-2402','BN43-2402')],
    "T37-2":[Quote(Decimal(0.25),'*Kits&Parts','http://www.kitsandparts.com','','T37-2','T37-2')],
    "T37-6":[Quote(Decimal(0.25),'*Kits&Parts','http://www.kitsandparts.com','','T37-6','T37-6')],
    "RD15HVF1":[Quote(Decimal(4.00),'*AliExpress','http://www.aliexpress.com','Mitsubishi','RD15HVF1','RD15HVF1')],
    "PCB":[Quote(Decimal(18.00),'*Tindie','http://www.tindie.com','Elecrow','','PCB')],
    "AK-C-C12":[Quote(Decimal(16.78),'*AliExpress','https://www.aliexpress.com/item/4-pieces-a-lot-top-sales-china-die-casting-aluminum-housing-55-106-150-mm/1243767700.html','Various','','Aluminum 100x100x50')],
    "PROG":[Quote(Decimal(3.00),'*EBay','http://www.ebay.com/itm/altera-Mini-Usb-Blaster-Cable-For-CPLD-FPGA-NIOS-JTAG-Altera-Programmer-/200943750380?hash=item2ec92e4cec:g:YyMAAOSw0fhXieqQ','Various','','USB Blaster')]
}

## Override for problems with OctoPart
overrides = {
    "OPA2677IDDA":[Quote(Decimal(3.90),'Digi-Key','http://www.digikey.com','','OPA2677IDDA','OPA2677IDDA')],
    "INA199A1DC":[Quote(Decimal(0.84),'Mouser','http://www.mouser.com','','INA199A1DC','INA199A1DC')],
}


class Octopart:
    db = pickledb.load("octopart.db", False)

    def __init__(self,key='&apikey=e2c08378'):
        self.key = key 

    def Dump(self):
        Octopart.db.dump()      

    def QueryWeb(self,mpns):
        if len(mpns) > 20:
            print "Octopart does not support more than 20 queries at a time for free"
            return


        queries = [{'mpn':l,'reference':l} for l in mpns]
        
        url = 'http://octopart.com/api/v3/parts/match?queries=%s' % urllib.quote(json.dumps(queries))
        url += self.key
        data = urllib.urlopen(url).read()
        response = json.loads(data)

        results = response['results']

        for result in results:
            Octopart.db.set(result['reference'],result)

        self.Dump()

    def DebugPrint(self,mpn):

        def rec(o,indent):
            if type(o) is dict:
                for k,v in o.iteritems():
                    if type(v) is dict or type(v) is list:
                        print indent*' ',k,':'
                        rec(v,indent+2)
                    else:
                        print indent*' ',k,':',v
            elif type(o) is list:
                for v in o:
                    rec(v,indent+2)
            else:
                print indent*' ',o
                
        v = self.db.get(mpn)
        if v == None:
            print "MPN",mpn,"does not exist in DB"
            for k in self.db.getall():
                print k
            return

        rec(v,0)

        return v

    def Quotes(self,mpn,quantity):

        ##print "Quote for",mpn,quantity
        quotes = []
        if mpn in overrides:
            print "WARNING: Using override for",mpn
            quotes.extend(overrides[mpn])
            return quotes
        else:
            v = self.db.get(mpn)

        if v == None: 
            ## Add any special quotes
            if mpn in special:
                quotes.extend(special[mpn])            
            ##print "MPN",mpn,"does not exist in DB"
            return quotes

        items = v['items']

        for item in items:
            manufacturer = item['manufacturer']['name']
            offers = item['offers']
            for offer in offers:
                ## Only accept if quotes are in USD
                if 'USD' not in offer['prices'].keys(): continue
                ## Only accept if enough parts are in stock
                if offer['in_stock_quantity'] < quantity: continue
                price = None
                for price_tuple in offer['prices']['USD']:
                    # Find correct price break
                    if price_tuple[0] > quantity: break
                    price = Decimal(price_tuple[1])
                ## Only accept if a valid price exists
                if not price: continue
                quote = Quote(price,offer['seller']['name'],offer['seller']['homepage_url'],manufacturer,offer['sku'],mpn)
                quotes.append(quote)

        ## Add any special quotes
        if mpn in special:
            quotes.extend(special[mpn])
        
        return quotes

octopart = Octopart()


class Component:
    def __init__(self,xml,hand,optionset):
        self.xml = xml
        self.option = ''
        self.notes = ''
        self.ext = ''
        self.key = ''


        self.value = self.xml.value.cdata

        try:
            fp = self.xml.footprint.cdata
            if '0603' in fp:
                self.ext = '0603'
            elif '0805' in fp:
                self.ext = '0805'
        except:
            pass

        try:
            for field in self.xml.fields.field:
                if field['name'] == 'Option':
                    self.option = field.cdata
                    ##print "Found option",self.option
                elif field['name'] == 'Notes':
                    self.notes = field.cdata
                elif field['name'] == 'Ext':
                    if self.ext != '':
                        self.ext = self.ext + ' ' + field.cdata
                    else:
                        self.ext = field.cdata
                    ##print "Found ext",self.ext
                elif field['name'] == 'Key':
                    self.key = field.cdata
                elif field['name'] == 'Hand' and hand:
                    self.key = field.cdata
        except:
            pass

    	## Handle option ORs
    	if '_' in self.option:
    		options = self.option.split('_')
    		for o in options:
    			no,nk = o.split(":")
    			if no in optionset:
    				self.option = no
    				self.key = nk
    				print "Forcing OR option",no,nk
    				break

        if '&' in self.option:
            options = self.option.split('&')
            if False in [o in optionset for o in options]:
                print "AND condition not met",options
            else:
                print "AND condtion met",options
                self.option = options[0]


        self.ref = self.xml['ref']
        if self.key == '':
            if self.ref[1].isdigit():
                self.key = (self.ref[0] + " " + self.value + " " + self.ext).strip()
            else:
                self.key = (self.ref[0:2] + " " + self.value + " " + self.ext).strip()
        elif self.ext != '':
            self.key = self.key + ' ' + self.ext

        ## Merge any requested jumpers into one class
        self.key = self.key.replace("J JNO","J JNC")


        ## Convert ref to sortable ascii,integer tuple
        if self.ref[1].isdigit():
            self.ref = self.ref[0],int(self.ref[1:])
        else:
            self.ref = self.ref[:2],int(self.ref[2:])

    def DebugStr(self):
        ##print "{0:15}{1:5}{2:20}{3:10}{4}".format(self.key,self.quantity,self.option,self.notes,self.mpns)
        return "{0:20} {1:20}{2:10}{3:5}".format(self.key,self.option,self.notes,self.ref)


class Part:

    def __init__(self,component):
        self.mpns = []
        self.spec = ''
        self.components = [component]
        ##print "Add component",component.DebugStr()

    def AddComponent(self,component):
        self.components.append(component)

    def AddPartSel(self,v):
        self.mpns = v['mpn']
        self.spec = v['spec'].encode('utf-8')
        self.pins = v['pins']
        self.assembly = v['assembly']

        if 'aliexpress' in v:
            self.aliexpress = v['aliexpress']
        else:
            self.aliexpress = None

        if 'sub' in v:
            self.sub = 'Y' if v['sub'] == 1 else 'N'
        else:
            self.sub = 'Y'

        if 'package' in v:
            self.package = LaTeXEscape(v['package'])
        else:
            if '0805' in self.spec or self.components[0].ext == '0805':
                self.package = '0805'
            elif '0603' in self.spec or self.components[0].ext == '0603': 
                self.package = '0603' 
            elif '1206' in self.spec:
                self.package = '1206'
            else:
                self.package = "Custom"

        if 'ecid' in v:
            self.ecid = v['ecid']
        else:
            self.ecid = ' '



    def Quantities(self,options):
        unique = set([])
        optional = 0
        for c in self.components:
            if c.option not in options:
                optional += 1
                unique.add(c.option)

        key = self.components[0].key
        if "NOBOM" in key:
            return (0,len(self.components),len(unique))
        else:
            return (1*(len(self.components)-optional),optional,len(unique))


    def Quotes(self,options,prefer=None):
        quantity = self.Quantities(options)
        quotes = []
        for mpn in self.mpns:
            quotes.extend(octopart.Quotes(mpn,quantity[0]))
        
        if prefer:
            quotes = [a for a in quotes if ((a.name in prefer) or (a.name[0] == '*'))]

        quotes.sort()

        return quotes

    def FirstRef(self,selectedoptions):
        for c in self.components:
            if c.option in selectedoptions:
                return c.ref
        return "REF ERROR"


    def WikiRefs(self,options,llen=20):

        refs = [c.ref for c in self.components if c.option in options]
        refs.sort()

        if refs == []: return ''

        res = '<p>'

        i = 0
        for ref in refs[:-1]:
            if i > llen:
                res = res + '</p><p>'
                i = 0
            ref = ref[0]+str(ref[1])
            res = res + ref + ","
            i = i + 1 + len(ref)

        if i > llen:
            res = res + '</p><p>'

        res = res + refs[-1][0] + str(refs[-1][1]) + '</p>'

        return res

    def LaTeXRefs(self,options,llen=16):

        refs = [c.ref for c in self.components if c.option in options]
        refs.sort()

        res = ''

        if refs == []: return res

        i = 0
        for ref in refs[:-1]:
            if i > llen:
                res = res + ' '
                i = 0
            ref = ref[0]+str(ref[1])
            res = res + ref + ","
            i = i + 1 + len(ref)

        if i > llen:
            res = res + ' '

        res = res + refs[-1][0] + str(refs[-1][1])

        return res

    def CSVRefs(self,options,llen=16):

        refs = [c.ref for c in self.components if c.option in options]
        refs.sort()

        res = ''

        if refs == []: return res

        i = 0
        for ref in refs[:-1]:
            if i > llen:
                res = res + '\n'
                i = 0
            ref = ref[0]+str(ref[1])
            res = res + ref + ","
            i = i + 1 + len(ref)

        if i > llen:
            res = res + '\n'

        res = res + refs[-1][0] + str(refs[-1][1])

        return res


    def LaTeXAssemblyLinks(self):

        if self.aliexpress:
            url = '\href{{http://www.aliexpress.com/wholesale?SearchText={0}}}{{{1}}}'.format(self.aliexpress,self.mpns[0])
        else:
            url = '\href{{http://www.octopart.com/search?q={0}}}{{{1}}}'.format(self.mpns[0],self.mpns[0])

        ##for mpn in self.mpns:
        ##    urls.append('\href{{http://www.octopart.com/search?q={0}}}{{{1}}}'.format(mpn,i) )
        
        return url

    def CSVAssemblyLinks(self):
        if self.aliexpress:
            url = '=HYPERLINK("http://www.aliexpress.com/wholesale?SearchText={0}")'.format(self.aliexpress)
        else:
            url = '=HYPERLINK("http://www.octopart.com/search?q={0}")'.format(self.mpns[0])

        ##for mpn in self.mpns:
        ##    urls.append('\href{{http://www.octopart.com/search?q={0}}}{{{1}}}'.format(mpn,i) )
        
        return url        

    def DNIRefs(self,options):
        return [c.ref for c in self.components if (c.option not in options and "NOBOM" not in c.key)]

    def Sort(self):
        self.components.sort(key=lambda x: x.ref)
 
    def DebugPrint(self):
        print "{0} {1:10} {2:20} {3}".format(self.components[0].DebugStr(),self.Quantities(),self.mpns,self.spec)

    def PrintOptional(self,opitons):
        for c in self.components:
            if c.option not in options: print c.ref,c.option




class BOM:

    def __init__(self,fn,hand=True,optionset=set([])):

        self.hand = hand
        ## Read components from KiCAD XML

        kicad = untangle.parse(fn)

        ml = kicad.export.components.comp

        self.parts = {}

        self.optionset = optionset
        ## Make sure no options components are selected
        self.optionset.add('')

        self.unusedoptions = set([])
  
        for comp in ml:
            c = Component(comp,self.hand,self.optionset)
            if c.key in self.parts:
                p = self.parts[c.key]
                p.AddComponent(c)
            else:
                self.parts[c.key] = Part(c)
            if c.option:
            	if c.option not in self.optionset:
            		self.unusedoptions.add(c.option)

        ## Pair with mpn
        f = open("parts.json")
        jo = json.load(f)
        f.close()

        for k,v in jo.iteritems():
            if k not in self.parts:
                print "WARNING: {0} in parts catalog but not used in design".format(k)
            else:
                ##if k == 'CASE':
                ##    print "Adding key",k,v
                p = self.parts[k]
                p.AddPartSel(v)

        for k,v in self.parts.iteritems():
            v.Sort()

        print "Selected Options:",self.optionset
        print
        print "Unselected Options:",self.unusedoptions

    def DebugPrint(self):
        keys = self.parts.keys()
        keys.sort()
        for k in keys:
            v = self.parts[k]
            v.DebugPrint()
            quotes = v.Quotes(self.optionset,prefer=['Mouser','Digi-Key'])
            ##print quotes
            for q in quotes: q.DebugPrint()

    def WikiPrint(self,prefer=None):
        keys = self.parts.keys()
        keys.sort(key=lambda x: self.parts[x].components[0].ref)
        total = Decimal(0.0)

        f = open("bom.md","w")

        # itemspartspins
        ipp = {
            'SMT':(0,0,0),
            'TH':(0,0,0),
            'MTH':(0,0,0)
        }

        print >>f, "|  Reference  | Spec    | Manufacturer | Part Number | Supplier | SKU | Price | Qty | Total |"
        print >>f, "| ----------- | ------- | ------------ | ----------- | -------- | --: | ----: | --: | ----: |"

        for k in keys:
            p = self.parts[k]

            c1 = p.WikiRefs(self.optionset)

            c3 = p.Quantities()[0]

            if c3 == 0: continue

            quotes = p.Quotes(self.optionset,prefer)
            if quotes != []:
                quote = quotes[0]
                c2 = quote.WikiLine()
                c4 = quote.price * c3
            else:
                print "ERROR: No quote for mpn",p.mpns,p.spec,p.components[0].ref,p.components[0].key
                continue

            items,parts,pins = ipp[p.assembly]
            ipp[p.assembly] = items+1,parts+c3,pins+(p.pins*c3)

            ##print "!!!",c1,p.spec,c2,c3,c4
            s = '| {0} | {1} | {2} | {3} | {4:.2f} |'.format(c1,p.spec,c2,c3,c4)
            total = total + c4

            print >>f,s

        print >>f," * Total Price: ${0:.2f}".format(total)
        print >>f," * Line Items"
        print >>f,"  * SMT: {0}".format(ipp['SMT'][0])
        print >>f,"  * Assembled TH: {0}".format(ipp['TH'][0])
        print >>f,"  * Manual TH: {0}".format(ipp['MTH'][0])
        print >>f,"  * Total: {0}".format(ipp['MTH'][0]+ipp['SMT'][0]+ipp['TH'][0])
        print >>f," * Parts"
        print >>f,"  * SMT: {0}".format(ipp['SMT'][1])
        print >>f,"  * Assembled TH: {0}".format(ipp['TH'][1])
        print >>f,"  * Manual TH: {0}".format(ipp['MTH'][1])
        print >>f,"  * Total: {0}".format(ipp['MTH'][1]+ipp['SMT'][1]+ipp['TH'][1])
        print >>f," * Pins"
        print >>f,"  * SMT: {0}".format(ipp['SMT'][2])
        print >>f,"  * Assembled TH: {0}".format(ipp['TH'][2])
        print >>f,"  * Manual TH: {0}".format(ipp['MTH'][2])
        print >>f,"  * Total: {0}".format(ipp['MTH'][2]+ipp['SMT'][2]+ipp['TH'][2])

    def LaTeXPrint(self,pre="",prefer=None):
        keys = self.parts.keys()
        keys.sort(key=lambda x: self.parts[x].FirstRef(self.optionset))
        ##keys.sort(key=lambda x: self.parts[x].components[0].ref)

        total = Decimal(0.0)

        f = open("bompre.dat","w")
        print >>f,pre
        f.close()

        f = open("bom.dat","w")

        # itemspartspins
        ipp = {
            'SMT':(0,0,0),
            'TH':(0,0,0),
            'MTH':(0,0,0)
        }

        dni = []

        for k in keys:
            p = self.parts[k]

            c1 = p.LaTeXRefs(self.optionset)
            dni.extend(p.DNIRefs(self.optionset))

            c3 = p.Quantities(self.optionset)[0]

            if c3 == 0: continue

            quotes = p.Quotes(self.optionset,prefer)
            if quotes != []:
                quote = quotes[0]
                c2 = quote.LaTeXLine()
                c4 = quote.price * c3
            else:
                print "ERROR: No quote for mpn",p.mpns,p.spec,p.components[0].ref,p.components[0].key
                continue

            items,parts,pins = ipp[p.assembly]
            ipp[p.assembly] = items+1,parts+c3,pins+(p.pins*c3)

            ##print "!!!",c1,p.spec,c2,c3,c4
            s = '{0} & {1} & {2} & {3} & {4:.2f} \\\\ \hline'.format(c1,LaTeXEscape(p.spec),c2,c3,c4)
            total = total + c4

            print >>f,s

        f.close()

        dni.sort()
        dni = [d[0]+str(d[1]) for d in dni]


        f = open("bompost.dat","w")

        print >>f,"\\noindent \\textbf{{Total Price:}} \\${0:.2f}\\\\".format(total)
        print >>f,"\\textbf{{Line Items:}} SMT:{0} Assembled TH:{1} Manual TH:{2} Total:{3}\\\\".format(ipp['SMT'][0],ipp['TH'][0],ipp['MTH'][0],ipp['MTH'][0]+ipp['SMT'][0]+ipp['TH'][0])
        print >>f,"\\textbf{{Parts:}} SMT:{0} Assembled TH:{1} Manual TH:{2} Total:{3}\\\\".format(ipp['SMT'][1],ipp['TH'][1],ipp['MTH'][1],ipp['MTH'][1]+ipp['SMT'][1]+ipp['TH'][1])
        print >>f,"\\textbf{{Pins:}} SMT:{0} Assembled TH:{1} Manual TH:{2} Total:{3}\\\\".format(ipp['SMT'][2],ipp['TH'][2],ipp['MTH'][2],ipp['MTH'][2]+ipp['SMT'][2]+ipp['TH'][2])
        

        dnis = ' '.join(dni).strip()        
        if dnis != '':
            print >>f,"\\textbf{{Do Not Include:}}"
            print >>f,dnis
            print >>f,"\\\\"

        f.close()


    def LaTeXAssemblyPrint(self,pre="",prefer=None):
        keys = self.parts.keys()
        ##print keys
        keys.sort(key=lambda x: self.parts[x].FirstRef(self.optionset))
        ##keys.sort(key=lambda x: self.parts[x].components[0].ref)

        total = Decimal(0.0)

        f = open("bompre.dat","w")
        print >>f,pre
        f.close()

        f = open("bom.dat","w")

        # itemspartspins
        ipp = {
            'SMT':(0,0,0),
            'TH':(0,0,0),
            'MTH':(0,0,0)
        }

        dni = []

        for k in keys:
            p = self.parts[k]
            print "---"
            print k
            print p.DNIRefs(self.optionset)
            dni.extend(p.DNIRefs(self.optionset))

            c8 = p.Quantities(self.optionset)[0]
            if c8 == 0: continue
			
            c1 = p.ecid
            c2 = LaTeXEscape(p.spec)
            c3 = p.LaTeXAssemblyLinks()
            c4 = p.sub
            c5 = p.LaTeXRefs(self.optionset)
            c6 = p.package
            c7 = p.pins

            items,parts,pins = ipp[p.assembly]
            ipp[p.assembly] = items+1,parts+c8,pins+(p.pins*c8)

            s = '{0} & {1} & {2} & {3} & {4} & {5} & {6} & {7} \\\\ \hline'.format(c1,c2,c3,c4,c5,c6,c7,c8)
            
            ##print c1,c2,c3,c4,c5,c6,c7,c8
            
            print >>f,s

        f.close()

        dni.sort()
        dni = [d[0]+str(d[1]) for d in dni]


        f = open("bompost.dat","w")

        ##print >>f,"\\noindent \\textbf{{Total Price:}} \\${0:.2f}\\\\".format(total)
        print >>f,"\\noindent \\textbf{{Line Items:}} SMT:{0} Assembled TH:{1}\\\\".format(ipp['SMT'][0],ipp['TH'][0])
        print >>f,"\\textbf{{Parts:}} SMT:{0} Assembled TH:{1}\\\\".format(ipp['SMT'][1],ipp['TH'][1])
        print >>f,"\\textbf{{Pins:}} SMT:{0} Assembled TH:{1}\\\\".format(ipp['SMT'][2],ipp['TH'][2])

        dnis = ' '.join(dni).strip()        
        if dnis != '':
            print >>f,"\\textbf{{Do Not Assemble:}}"
            print >>f,dnis
            print >>f,"\\\\"

        f.close()

    def CSVAssemblyPrint(self,prefer=None):
        keys = self.parts.keys()
        keys.sort(key=lambda x: self.parts[x].FirstRef(self.optionset))

        f = open("bomassembly.csv","w")

        s = '"Part ID", "Description", "Part Number", "Substitution\nOkay", "Designators", "Footprint", "Pins", "Quantity", "Part Reference Link"'
        print >>f,s

        # itemspartspins
        ipp = {
            'SMT':(0,0,0),
            'TH':(0,0,0),
            'MTH':(0,0,0)
        }

        dni = []

        for k in keys:
            p = self.parts[k]

            dni.extend(p.DNIRefs(self.optionset))

            c8 = p.Quantities(self.optionset)[0]
            if c8 == 0: continue

            c1 = p.ecid
            c2 = p.spec
            c3 = p.mpns[0] 
            c4 = p.sub
            c5 = p.CSVRefs(self.optionset)
            c6 = p.package
            c7 = p.pins
            c9 = p.CSVAssemblyLinks()

            items,parts,pins = ipp[p.assembly]
            ipp[p.assembly] = items+1,parts+c8,pins+(p.pins*c8)

            s = '{0}, "{1}", "{2}", {3}, "{4}", "{5}", {6}, {7}, "{8}"'.format(c1,c2,c3,c4,c5,c6,c7,c8,c9)
            
            ##print c1,c2,c3,c4,c5,c6,c7,c8
            
            print >>f,s

        print >>f,""

        dni.sort()
        dni = [d[0]+str(d[1]) for d in dni]

        print >>f,'"Line Items SMT:", " ", {0}, "Line Items TH:", " ", {1}'.format(ipp['SMT'][0],ipp['TH'][0])
        print >>f,'"Parts SMT:", " ", {0}, "Parts TH:", " ", {1}'.format(ipp['SMT'][1],ipp['TH'][1])
        print >>f,'"Pins SMT:", " ", {0}, "Pins TH:", " ", {1}'.format(ipp['SMT'][2],ipp['TH'][2])

        print >>f,""

        ##dnis = ' '.join(dni).strip()  
        llen = 30
        res = ''      
        if dni != []:
            print >>f,"Do Not Assemble:"

            i = 0
            for ref in dni[:-1]:
                if i > llen:
                    res = res + '\n'
                    i = 0
                ref = ref[0]+str(ref[1])
                res = res + ref + ","
                i = i + 1 + len(ref)
    
            if i > llen:
                res = res + '\n'
    
            res = res + dni[-1][0] + str(dni[-1][1])

            print >>f,'"{0}"'.format(res)

        f.close()


    def CSVSeeedStudioAssemblyPrint(self,prefer=None):
        keys = self.parts.keys()
        keys.sort(key=lambda x: self.parts[x].FirstRef(self.optionset))

        f = open("seedstudiobomassembly.csv","w")

        ##s = '"Part ID", "Description", "Part Number", "Substitution\nOkay", "Designators", "Footprint", "Pins", "Quantity", "Part Reference Link"'
        s = '"Designator"; "MPN/Seeed SKU"; "Qty"'
        print >>f,s

        # itemspartspins
        ipp = {
            'SMT':(0,0,0),
            'TH':(0,0,0),
            'MTH':(0,0,0)
        }

        dni = []

        for k in keys:
            p = self.parts[k]

            dni.extend(p.DNIRefs(self.optionset))

            c8 = p.Quantities(self.optionset)[0]
            if c8 == 0: continue

            c1 = p.ecid
            c2 = p.spec
            c3 = p.mpns[0] 
            c4 = p.sub
            c5 = p.CSVRefs(self.optionset)
            c6 = p.package
            c7 = p.pins
            c9 = p.CSVAssemblyLinks()

            items,parts,pins = ipp[p.assembly]
            ipp[p.assembly] = items+1,parts+c8,pins+(p.pins*c8)

            ##s = '{0}, "{1}", "{2}", {3}, "{4}", "{5}", {6}, {7}, "{8}"'.format(c1,c2,c3,c4,c5,c6,c7,c8,c9)
            s = '{0}; "{1}"; "{2}"'.format(c5,c3,c8)
            
            ##print c1,c2,c3,c4,c5,c6,c7,c8
            
            print >>f,s

        print >>f,""

        dni.sort()
        dni = [d[0]+str(d[1]) for d in dni]

        print >>f,'"Line Items SMT:", {0}, "Line Items TH:", {1}'.format(ipp['SMT'][0],ipp['TH'][0])
        print >>f,'"Parts SMT:", {0}, "Parts TH:", {1}'.format(ipp['SMT'][1],ipp['TH'][1])
        print >>f,'"Pins SMT:", {0}, "Pins TH:", {1}'.format(ipp['SMT'][2],ipp['TH'][2])

        print >>f,""

        dnis = ' '.join(dni).strip()        
        if dnis != '':
            print >>f,"Do Not Assemble:"
            print >>f,dnis

        f.close()




    def OctoPartUpdatePrices(self):

        mpns = []
        for key,part in self.parts.iteritems():
            mpns.extend(part.mpns)

        ## 20 parts at a time
        for i in range(0,len(mpns),20):
            sl = mpns[i:i+20]
            octopart.QueryWeb(sl)

    def PrintOptional(self):
        keys = self.parts.keys()
        keys.sort(key=lambda x: self.parts[x].components[0].ref)

        for key in keys:
            p = self.parts[key]
            p.PrintOptional(self.optionset)


  



