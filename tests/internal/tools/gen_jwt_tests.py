#!/usr/bin/env python3
"""Keys and tokens for std/encoding/jwt, made by PyJWT and the cryptography
package.

    pip install pyjwt cryptography
    tests/internal/tools/gen_jwt_tests.py

std/encoding/jwt/testdata/ gets the keys (PKCS #8 and SubjectPublicKeyInfo PEM
files: an RSA 2048-bit key, P-256, P-384 and Ed25519 keys, and a second P-256
key that signed nothing) and cases.json: tokens signed by PyJWT, each with the
algorithm and key to verify it with, the claim checks to make at the time
"now" (when the file was made), and PyJWT's verdict as std/encoding/jwt's
error name ("ok" if it accepts; where std/encoding/jwt is stricter, as with
a header naming "alg" twice, "result" says so and "pyjwt" has PyJWT's verdict). For the deterministic algorithms (HMAC,
RSASSA-PKCS1-v1_5, EdDSA) "payload" is the exact JSON PyJWT signed, so the
test can sign it again and compare the tokens byte for byte.
"""
import base64
import hashlib
import hmac
import json
import os
import time

import jwt
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec, ed25519, rsa

out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'std', 'encoding', 'jwt', 'testdata')
os.makedirs(out_dir, exist_ok=True)

keys = {
    'rsa': rsa.generate_private_key(public_exponent=65537, key_size=2048),
    'p256': ec.generate_private_key(ec.SECP256R1()),
    'p256-other': ec.generate_private_key(ec.SECP256R1()),
    'p384': ec.generate_private_key(ec.SECP384R1()),
    'ed25519': ed25519.Ed25519PrivateKey.generate(),
}
for name, k in keys.items():
    open(os.path.join(out_dir, name + '-private.pem'), 'wb').write(k.private_bytes(
        serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8, serialization.NoEncryption()))
    open(os.path.join(out_dir, name + '-public.pem'), 'wb').write(k.public_key().public_bytes(
        serialization.Encoding.PEM, serialization.PublicFormat.SubjectPublicKeyInfo))

# HMAC secrets as long as the hash (RFC 7518 §3.2)
SECRETS = {'HS256': 'a-string-secret-at-least-256-bits-long',
           'HS384': 'a-valid-string-secret-that-is-at-least-384-bits-long',
           'HS512': 'a-valid-string-secret-that-is-at-least-512-bits-long-which-is-very-long'}
KEY_OF = {'RS': 'rsa', 'PS': 'rsa', 'ES256': 'p256', 'ES384': 'p384', 'EdDSA': 'ed25519'}


def key_name(alg):
    if alg.startswith('HS'):
        return 'secret:' + SECRETS[alg]
    return KEY_OF.get(alg, KEY_OF.get(alg[:2]))


def signing_key(alg):
    return SECRETS[alg] if alg.startswith('HS') else keys[key_name(alg)]


def b64(b):
    return base64.urlsafe_b64encode(b).rstrip(b'=').decode()


now = int(time.time())
ERRORS = {jwt.ExpiredSignatureError: 'Expired', jwt.ImmatureSignatureError: 'NotYetValid',
          jwt.InvalidAudienceError: 'InvalidAudience', jwt.InvalidIssuerError: 'InvalidIssuer',
          jwt.MissingRequiredClaimError: 'MissingClaim', jwt.InvalidSignatureError: 'InvalidSignature',
          jwt.InvalidAlgorithmError: 'AlgorithmMismatch', jwt.DecodeError: 'InvalidToken'}


def verdict(token, alg, key, audience=None, issuer=None, leeway=0):
    """PyJWT's opinion, as an error name."""
    if not alg.startswith('HS'):
        key = keys[key].public_key()
    else:
        key = key[len('secret:'):]
    try:
        jwt.decode(token, key, algorithms=[alg], audience=audience, issuer=issuer, leeway=leeway)
        return 'ok'
    except jwt.PyJWTError as e:
        for cls, name in ERRORS.items():
            if isinstance(e, cls):
                return name
        return 'InvalidToken'  # (InvalidTokenError itself: an unsupported critical extension)


cases = []


def add(token, alg, key=None, audience=None, issuer=None, leeway=0, payload=None, result=None, note=''):
    key = key or key_name(alg)
    got = verdict(token, alg, key, audience, issuer, leeway)
    case = {'note': note, 'token': token, 'alg': alg, 'key': key, 'audience': audience or '',
            'issuer': issuer or '', 'leeway': leeway, 'result': result or got}
    if result is not None and result != got:  # (stricter than PyJWT, stated by hand)
        case['pyjwt'] = got
    if payload is not None:
        case['payload'] = payload
    cases.append(case)


claims = {'sub': '1234567890', 'name': 'John Doe é☃', 'admin': True, 'iat': now - 100, 'exp': now + 3600}
payload_json = json.dumps(claims, separators=(',', ':'))  # what PyJWT signs (ensure_ascii, as here)
for alg in ['HS256', 'HS384', 'HS512', 'RS256', 'RS384', 'RS512', 'PS256', 'PS384', 'PS512', 'ES256', 'ES384',
            'EdDSA']:
    tok = jwt.encode(claims, signing_key(alg), algorithm=alg)
    det = alg[:2] in ('HS', 'RS') or alg == 'EdDSA'
    add(tok, alg, payload=payload_json if det else None, note='signed by PyJWT')
    tok = jwt.encode(claims, signing_key(alg), algorithm=alg, headers={'kid': 'key-1'})
    add(tok, alg, payload=payload_json if det else None, note='with a kid')

# claims, checked at "now"
S = SECRETS['HS256']


def hs(c, **kw):
    return jwt.encode(c, S, algorithm='HS256', **kw)


add(hs({'exp': now - 100}), 'HS256', note='expired')
add(hs({'exp': now - 30}), 'HS256', leeway=60, note='expired, within the leeway')
add(hs({'exp': now + 100.5}), 'HS256', note='a fractional exp')
add(hs({'nbf': now + 100}), 'HS256', note='not yet valid')
add(hs({'nbf': now + 30}), 'HS256', leeway=60, note='not yet valid, within the leeway')
add(hs({'nbf': now - 30, 'exp': now + 30}), 'HS256', note='in its window')
add(hs({'iat': now + 100}), 'HS256', note='issued in the future')
add(hs({'aud': 'api'}), 'HS256', audience='api', note='audience')
add(hs({'aud': ['web', 'api']}), 'HS256', audience='api', note='audience in a list')
add(hs({'aud': ['web', 'app']}), 'HS256', audience='api', note='another audience')
add(hs({'aud': 'api'}), 'HS256', note='an audience nobody expected')
add(hs({'sub': 'x'}), 'HS256', audience='api', note='no audience')
add(hs({'aud': ['web', 3]}), 'HS256', audience='web', note='an audience that is not a string')
add(hs({'iss': 'https://issuer.example'}), 'HS256', issuer='https://issuer.example', note='issuer')
add(hs({'iss': 'https://other.example'}), 'HS256', issuer='https://issuer.example', note='another issuer')
add(hs({'sub': 'x'}), 'HS256', issuer='https://issuer.example', note='no issuer')
add(hs({'iss': 'i', 'aud': 'a', 'nbf': now - 5, 'iat': now - 5, 'exp': now + 5}), 'HS256', audience='a',
    issuer='i', note='everything')

# attacks and damage
good = hs({'sub': 'x'})
h, p, s = good.split('.')
add(h + '.' + b64(b'{"sub":"y"}') + '.' + s, 'HS256', note='a changed payload')
add(h + '.' + p + '.' + s[:-2] + ('AA' if s[-2:] != 'AA' else 'BA'), 'HS256', note='a changed signature')
add(h + '.' + p + '.', 'HS256', note='no signature')
add(jwt.encode({'sub': 'x'}, None, algorithm='none'), 'HS256', result='AlgorithmMismatch', note='alg none')
rsa_pub_pem = open(os.path.join(out_dir, 'rsa-public.pem'), 'rb').read()
hdr = b64(b'{"alg":"HS256","typ":"JWT"}')
body = b64(b'{"sub":"admin"}')
forged = hdr + '.' + body + '.' + b64(hmac.new(rsa_pub_pem, (hdr + '.' + body).encode(), hashlib.sha256).digest())
add(forged, 'RS256', note='HS256 keyed with the RSA public key (algorithm confusion)')
add(jwt.encode({'sub': 'x'}, keys['rsa'], algorithm='RS256'), 'RS384', key='rsa', note='RS256 where RS384 is expected')
add(jwt.encode({'sub': 'x'}, keys['p256'], algorithm='ES256'), 'ES256', key='p256-other', note='another key')
add(jwt.encode({'sub': 'x'}, keys['rsa'], algorithm='PS256'), 'RS256', key='rsa', note='PS256 where RS256 is expected')
crit = b64(b'{"alg":"HS256","crit":["exp"],"exp":1}')
add(crit + '.' + body + '.' + b64(hmac.new(S.encode(), (crit + '.' + body).encode(), hashlib.sha256).digest()),
    'HS256', result='InvalidToken', note='a critical header parameter')
dup = b64(b'{"alg":"none","alg":"HS256"}')
add(dup + '.' + body + '.' + b64(hmac.new(S.encode(), (dup + '.' + body).encode(), hashlib.sha256).digest()),
    'HS256', result='InvalidToken', note='alg given twice')
arr = b64(b'["sub"]')
add(hdr + '.' + arr + '.' + b64(hmac.new(S.encode(), (hdr + '.' + arr).encode(), hashlib.sha256).digest()),
    'HS256', result='InvalidToken', note='claims that are not an object')
padded = good.split('.')[0] + '=.' + p + '.' + s
add(padded, 'HS256', result='InvalidToken', note='base64 padding')

json.dump({'now': now, 'cases': cases}, open(os.path.join(out_dir, 'cases.json'), 'w'), indent=1, ensure_ascii=False)
print('%d cases in %s' % (len(cases), os.path.normpath(out_dir)))
