#!/usr/bin/env python3
"""Generates std/encoding/xml/testdata/cases.txt: XML documents, each with libxml2's
canonical form of it (xmllint --c14n), or "error" when libxml2 rejects it.

    cmd/axiom/internal/tests/tools/gen_xml_tests.py

Each case is a header line `case <n> <input bytes> <expected bytes or -1>`,
then the input and the expected output, each followed by a newline. Cases
where this library deliberately differs from libxml2 (external entities,
which are never loaded; libxml2's URI checks on namespace names) are left
out of the corpus.
"""
import os
import subprocess
import tempfile

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..', 'std', 'encoding', 'xml', 'testdata', 'cases.txt')
docs = [
 '<a/>', '<a></a>', '<a>text</a>', '<a b="1" c=\'2\'>x</a>', '<?xml version="1.0"?><a/>',
 '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<a/>', '<a><b><c/></b>tail</a>',
 '<a>&lt;&gt;&amp;&apos;&quot;</a>', '<a>&#65;&#x42;&#x1F600;</a>', '<a b="&lt;&#10;&#9;x"/>',
 '<a b="line\nbreak\ttab"/>', '<!-- c --><a/><!-- d -->', '<?p x?><a/><?q?>', '<a><![CDATA[<b>&amp;]]></a>',
 '<a>]]></a>', '<a b="1" b="2"/>', '<a><b></a></b>', '<a>', '</a>', '<a/><b/>', 'text<a/>', '<a/>text',
 '<a x="<"/>', '<a x=1/>', '<a x="1"y="2"/>', '<a><!-- a -- b --></a>', '<a><!-- a ---></a>',
 '<!DOCTYPE a><a/>', '<!DOCTYPE a [<!ENTITY e "hi">]><a>&e;</a>', '<!DOCTYPE a [<!ENTITY e "<b>x</b>">]><a>&e;</a>',
 '<!DOCTYPE a [<!ENTITY e "&f;"><!ENTITY f "ok">]><a>&e;</a>', '<!DOCTYPE a [<!ENTITY e "&e;">]><a>&e;</a>',
 '<a>&undefined;</a>', '<!DOCTYPE a [<!ATTLIST a x CDATA "def" y NMTOKENS " p  q ">]><a/>',
 '<!DOCTYPE a [<!ATTLIST a y NMTOKENS #IMPLIED>]><a y="  p   q  "/>',
 '<!DOCTYPE a [<!ELEMENT a (#PCDATA|b)*><!ATTLIST a x (u|v) "u"><!NOTATION n SYSTEM "x">]><a/>',
 '<!DOCTYPE a SYSTEM "a.dtd"><a/>', '<!DOCTYPE a PUBLIC "-//X//Y" "a.dtd" [<!ENTITY z "1">]><a>&z;</a>',
 '<!DOCTYPE a [<!ENTITY ext SYSTEM "ext.xml">]><a>&ext;</a>', '<!DOCTYPE a [<!ENTITY % p "<!ENTITY q \'qq\'>">%p;]><a>&q;</a>',
 '<a xmlns="http://x"><b/></a>', '<p:a xmlns:p="http://p"><p:b p:c="1"/></p:a>', '<p:a/>',
 '<a xmlns:p="http://p" xmlns:q="http://p"><b p:x="1" q:x="2"/></a>', '<a xmlns:p="http://p"><b xmlns:p="http://p"/></a>',
 '<a xmlns="http://x"><b xmlns=""/></a>', '<a xmlns:p=""/>', '<a xmlns:xml="http://www.w3.org/XML/1998/namespace" xml:lang="en"/>',
 '<a xmlns:xmlns="http://y"/>', '<a:b:c xmlns:a="http://a"/>', '<a z="1" xmlns:b="http://b" b:y="2" a="3" xmlns="http://d"/>',
 '<a>\r\nx\ry</a>', '<a b="\r\n"/>', '<a>\x01</a>', '<a>&#0;</a>', '<a>&#xD800;</a>', '<a>&#x110000;</a>',
 '<?xml version="1.0"?>\n<?xml version="1.0"?><a/>', ' <?xml version="1.0"?><a/>', '<?xml version="2.0"?><a/>',
 '<?xml encoding="UTF-8"?><a/>', '<?xml version="1.0" standalone="maybe"?><a/>', '<?XML version="1.0"?><a/>',
 '<a><?xml x?></a>', '<a><?pi?></a>', '<a><?pi  data  ?></a>', '<a>é中\U0001f600</a>',
 '<élément attrï="v"/>', '<1a/>', '<a-b.c_d/>', '<a><b/><c/>t<d>u</d></a>',
 '<!DOCTYPE a [<!ENTITY a "&#38;#38;">]><a>&a;</a>', '<!DOCTYPE a [<!ENTITY lt "&#38;#60;">]><a>&lt;</a>',
 '<!DOCTYPE a [<!ENTITY e "x<b>">]><a>&e;</b></a>', '<!DOCTYPE a [<!ENTITY e "a">]><a b="&e;&e;"/>',
 '<!DOCTYPE a [<!ENTITY e "<">]><a b="&e;"/>', '<a b="&amp;&#x26;"/>', '<a>  \n  </a>', '<a/>\n\n<!--x-->\n',
 '<!DOCTYPE a [\n<!-- c -->\n<?pi x?>\n]><a/>', '<!DOCTYPE a [<!ENTITY e "v"><!ENTITY e "w">]><a>&e;</a>',
 '<a>&#x9;&#xA;&#xD;</a>', '<a b="&#x9;&#xA;&#xD;"/>', '<a><![CDATA[]]></a>', '<a><![CDATA[x]]]]><![CDATA[>y]]></a>',
 '<!DOCTYPE a [<!ATTLIST a xmlns:p CDATA "http://q">]><a p:x="1"/>', '<a b = "1" ></a >', '<a></ a>',
 '<!DOCTYPE a [<!ENTITY % pe "x">]><a/>', '<!DOCTYPE a [<!ENTITY e SYSTEM "u" NDATA n>]><a>&e;</a>',
]
docs = [d for d in docs if 'SYSTEM "ext' not in d and 'SYSTEM "u" NDATA' not in d]
with open(OUT, 'wb') as out, tempfile.TemporaryDirectory() as tmp:
    out.write(b'# XML cases with xmllint --c14n output (cmd/axiom/internal/tests/tools/gen_xml_tests.py)\n')
    for i, d in enumerate(docs):
        f = os.path.join(tmp, 'c.xml')
        data = d.encode('utf-8')
        open(f, 'wb').write(data)
        x = subprocess.run(['xmllint', '--c14n', '--nonet', f], capture_output=True)
        ok = x.returncode == 0 and b'error' not in x.stderr.lower()
        exp = x.stdout if ok else None
        out.write(b'case %d %d %d\n' % (i, len(data), -1 if exp is None else len(exp)))
        out.write(data + b'\n')
        if exp is not None:
            out.write(exp + b'\n')
print(OUT, len(docs), 'cases')
