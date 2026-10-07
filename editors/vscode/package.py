#!/usr/bin/env python3
"""Packages the Axiom extension as a .vsix (what `vsce package` makes), without
needing Node.js or npm:

    python editors/vscode/package.py            # writes editors/vscode/axiom-<version>.vsix
    code --install-extension editors/vscode/axiom-<version>.vsix
"""
import json
import os
import sys
import zipfile
from xml.sax.saxutils import escape

HERE = os.path.dirname(os.path.abspath(__file__))
FILES = [
    'package.json',
    'extension.js',
    'language-configuration.json',
    'README.md',
    'syntaxes/axiom.tmLanguage.json',
    'syntaxes/axiom.markdown.tmLanguage.json',
]


def main():
    with open(os.path.join(HERE, 'package.json'), encoding='utf-8') as f:
        pkg = json.load(f)
    name, publisher, version = pkg['name'], pkg['publisher'], pkg['version']
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, f'{name}-{version}.vsix')
    manifest = f'''<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata>
    <Identity Language="en-US" Id="{escape(name)}" Version="{escape(version)}" Publisher="{escape(publisher)}"/>
    <DisplayName>{escape(pkg['displayName'])}</DisplayName>
    <Description xml:space="preserve">{escape(pkg['description'])}</Description>
    <Categories>{escape(','.join(pkg.get('categories', [])))}</Categories>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="{escape(pkg['engines']['vscode'])}"/>
    </Properties>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code"/>
  </Installation>
  <Dependencies/>
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/>
    <Asset Type="Microsoft.VisualStudio.Services.Content.Details" Path="extension/README.md" Addressable="true"/>
  </Assets>
</PackageManifest>
'''
    content_types = '''<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".json" ContentType="application/json"/>
  <Default Extension=".js" ContentType="application/javascript"/>
  <Default Extension=".md" ContentType="text/markdown"/>
  <Default Extension=".vsixmanifest" ContentType="text/xml"/>
</Types>
'''
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', content_types)
        z.writestr('extension.vsixmanifest', manifest)
        for f in FILES:
            z.write(os.path.join(HERE, f), 'extension/' + f)
    print(out)


if __name__ == '__main__':
    main()
