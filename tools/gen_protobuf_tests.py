#!/usr/bin/env python3
"""Test data for std/encoding/protobuf, from the Python protobuf package.

    pip install protobuf
    tools/gen_protobuf_tests.py

descriptor.bin is the FileDescriptorProto of google/protobuf/descriptor.proto
(a large real message), and facts.txt what it holds. scalars.txt has one
field per line, `<type> <field> <value> <hex>`, encoded by the library.
"""
import os

from google.protobuf import __version__
from google.protobuf import descriptor_pb2
from google.protobuf.internal import encoder

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'encoding', 'protobuf', 'testdata')
os.makedirs(out, exist_ok=True)

fd = descriptor_pb2.FileDescriptorProto()
descriptor_pb2.DESCRIPTOR.CopyToProto(fd)
open(os.path.join(out, 'descriptor.bin'), 'wb').write(fd.SerializeToString())
with open(os.path.join(out, 'facts.txt'), 'w') as f:
    f.write('# protobuf %s (tools/gen_protobuf_tests.py)\n' % __version__)
    f.write('name %s\n' % fd.name)
    f.write('package %s\n' % fd.package)
    f.write('messages %d\n' % len(fd.message_type))
    f.write('fields %d\n' % sum(len(m.field) for m in fd.message_type))
    f.write('enums %d\n' % len(fd.enum_type))
    for m in fd.message_type:
        f.write('message %s %d\n' % (m.name, len(m.field)))

ENC = {
    'int32': encoder.Int32Encoder, 'int64': encoder.Int64Encoder, 'uint32': encoder.UInt32Encoder,
    'uint64': encoder.UInt64Encoder, 'sint32': encoder.SInt32Encoder, 'sint64': encoder.SInt64Encoder,
    'bool': encoder.BoolEncoder, 'fixed32': encoder.Fixed32Encoder, 'fixed64': encoder.Fixed64Encoder,
    'sfixed32': encoder.SFixed32Encoder, 'sfixed64': encoder.SFixed64Encoder, 'float': encoder.FloatEncoder,
    'double': encoder.DoubleEncoder, 'string': encoder.StringEncoder,
}
CASES = [
    ('int32', 1, [0, 1, 150, -1, -2, 2147483647, -2147483648]),
    ('int64', 2, [0, 300, -1, 9223372036854775807, -9223372036854775808]),
    ('uint32', 3, [0, 4294967295]),
    ('uint64', 4, [0, 18446744073709551615]),
    ('sint32', 5, [0, -1, 1, -2, 2147483647, -2147483648]),
    ('sint64', 6, [0, -1, 1, 9223372036854775807, -9223372036854775808]),
    ('bool', 7, [False, True]),
    ('fixed32', 8, [0, 4294967295, 12345]),
    ('fixed64', 9, [0, 18446744073709551615]),
    ('sfixed32', 10, [-1, 2147483647, -2147483648]),
    ('sfixed64', 11, [-1, -9223372036854775808]),
    ('float', 12, [0.5, -1.25, 3.0e38]),
    ('double', 13, [0.1, -2.5e300]),
    ('string', 536870911, ['', 'testing', 'café']),
]
with open(os.path.join(out, 'scalars.txt'), 'w') as f:
    f.write('# protobuf %s (tools/gen_protobuf_tests.py)\n' % __version__)
    for typ, num, values in CASES:
        enc = ENC[typ](num, False, False)
        for v in values:
            parts = []
            enc(parts.append, v, True)
            b = b''.join(parts)
            shown = v
            if typ == 'bool':
                shown = int(v)
            elif typ in ('float', 'double'):
                shown = repr(v)
            elif typ == 'string':
                shown = v.encode('utf-8').hex() or '-'
            f.write('%s %d %s %s\n' % (typ, num, shown, b.hex()))
print('ok')
