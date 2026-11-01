#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Runner de tests - Maquina Virtual Parte II (MV2 2026)
=====================================================
Uso:   python run_tests.py                (usa vmt / vmx del PATH o de esta carpeta)
       python run_tests.py --vmt ./vmt.exe --vmx ./vmx.exe
       python run_tests.py --solo T02 T07  (corre solo esos tests)

Cada test .asm se autoverifica: imprime "-> OK" o "-> FALLA" por chequeo y
al final "RESULTADO: TODO OK". Este script ademas verifica errores esperados
(stack overflow/underflow, memoria insuficiente), el archivo .vmi y el -d.
La salida completa de cada corrida queda en la carpeta resultados/.
"""
import argparse, os, re, shutil, subprocess, sys

AQUI = os.path.dirname(os.path.abspath(__file__))
os.chdir(AQUI)
OUT = os.path.join(AQUI, 'resultados')
TIMEOUT = 15

ap = argparse.ArgumentParser()
ap.add_argument('--vmt', default=None, help='ejecutable del traductor')
ap.add_argument('--vmx', default=None, help='ejecutable de la VM')
ap.add_argument('--vmt-args', default='{asm} {vmx} v={v}',
                help='plantilla de argumentos de vmt (default: "{asm} {vmx} v={v}")')
ap.add_argument('--solo', nargs='*', default=None, help='prefijos de tests a correr (ej: T02 T07)')
a = ap.parse_args()

def buscar(nombre):
    for c in (nombre, nombre + '.exe'):
        p = os.path.join(AQUI, c)
        if os.path.isfile(p): return p
    return shutil.which(nombre)

VMT = a.vmt or buscar('vmt')
VMX = a.vmx or buscar('vmx')
if not VMT or not VMX:
    sys.exit('No encuentro vmt/vmx. Copielos a esta carpeta o use --vmt y --vmx.')

os.makedirs(OUT, exist_ok=True)
resultados = []

def correr(cmd, stdin='', etiqueta=''):
    try:
        p = subprocess.run(cmd, input=stdin.encode(), stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=TIMEOUT)
        txt = p.stdout.decode('latin1', 'replace')
    except subprocess.TimeoutExpired as e:
        txt = (e.stdout or b'').decode('latin1', 'replace') + '\n<<TIMEOUT: la VM no termino>>'
    with open(os.path.join(OUT, etiqueta + '.txt'), 'w', encoding='utf-8') as f:
        f.write('$ ' + ' '.join(cmd) + '\n' + txt)
    return txt

def traducir(base, version=2):
    args = a.vmt_args.format(asm=base + '.asm', vmx=base + '.vmx', v=version).split()
    txt = correr([VMT] + args, etiqueta=base + '__vmt')
    ok = os.path.isfile(base + '.vmx')
    if not ok: print('  [!] no se genero %s.vmx:\n%s' % (base, txt))
    return ok

def registrar(id_, desc, ok, motivo=''):
    resultados.append((id_, desc, ok, motivo))
    print('  %-4s %-8s %s%s' % ('OK' if ok else 'FALLA', id_, desc, '' if ok else '  -> ' + motivo))

def chequear(id_, desc, txt, contiene=(), no_contiene=(), regex=None, todo_ok=False):
    falt = [c for c in contiene if c.lower() not in txt.lower()]
    sobra = [c for c in no_contiene if c.lower() in txt.lower()]
    motivos = []
    if todo_ok:
        if 'RESULTADO: TODO OK' not in txt: motivos.append('no imprimio "RESULTADO: TODO OK"')
        fallas = [l.strip() for l in txt.splitlines() if 'FALLA' in l and 'RESULTADO' not in l]
        if fallas: motivos.append('chequeos en FALLA: ' + '; '.join(fallas))
    if falt: motivos.append('falta en la salida: ' + ', '.join(repr(x) for x in falt))
    if sobra: motivos.append('no deberia aparecer: ' + ', '.join(repr(x) for x in sobra))
    if regex and not re.search(regex, txt, re.I): motivos.append('no aparece mensaje /%s/' % regex)
    if 'TIMEOUT' in txt: motivos.append('timeout')
    registrar(id_, desc, not motivos, ' | '.join(motivos))

def quiere(pref):
    return a.solo is None or any(pref.startswith(s) for s in a.solo)

def vmx(*args, stdin='', etq):
    return correr([VMX] + list(args), stdin=stdin, etiqueta=etq)

print('VMT =', VMT); print('VMX =', VMX); print()

simples = [('T01', 'T01_estado_inicial', 'Estado inicial: pila de main, tabla, registros de segmento'),
           ('T02', 'T02_push_pop', 'PUSH/POP: big endian, signo, truncado'),
           ('T03', 'T03_call_ret', 'CALL/RET, anidado, recursion con BP'),
           ('T04', 'T04_sectores_modificadores', 'Sectores AL/AH/AX y modificadores b/w/l')]
for id_, base, desc in simples:
    if quiere(id_) and traducir(base):
        t = vmx(base + '.vmx', etq=base)
        chequear(id_, desc, t, todo_ok=True, no_contiene=['ERROR: la VM siguio'])

if quiere('T05') and traducir('T05_sys_strings'):
    t = vmx('T05_sys_strings.vmx', stdin='hola\nabcdefgh\n', etq='T05_sys_strings')
    chequear('T05', 'SYS 0/3/4 y SYS F ignorado sin .vmi', t, todo_ok=True,
             contiene=['Hola desde el Const Segment', 'Eco: hola', 'Eco: abcde'], no_contiene=['Eco: abcdef'])
    registrar('T05.v', 'SYS 0: verificar A OJO que la pantalla se limpio (no se automatiza)', True)

if quiere('T06') and traducir('T06_parametros'):
    t = vmx('T06_parametros.vmx', '-p', 'prueba', '01', '2', etq='T06_parametros')
    chequear('T06', 'Param Segment / argc / argv', t, todo_ok=True,
             contiene=['T06_parametros.vmx\n', 'prueba\n', '01\n', '2\n'])

if quiere('T07') and traducir('T07_breakpoint_imagen'):
    B = 'T07_breakpoint_imagen'
    for f in ('T07.vmi', 'T07_m32.vmi'):
        if os.path.exists(f): os.remove(f)
    t = vmx(B + '.vmx', etq=B + '_a_sin_vmi')
    chequear('T07.a', 'Sin .vmi los breakpoints se ignoran', t, todo_ok=True, contiene=['Paso 3'])
    t = vmx(B + '.vmx', 'T07.vmi', stdin='g\ng\n', etq=B + '_b_go')
    chequear('T07.b', "Con .vmi + 'g' en cada pausa termina normal", t, todo_ok=True, contiene=['Paso 3'])
    ok = os.path.isfile('T07.vmi'); motivo = 'no se creo T07.vmi'
    if ok:
        d = open('T07.vmi', 'rb').read()
        esperado = 8 + 128 + 32 + 16 * 1024
        checks = [(d[:5] == b'VMI26', 'identificador != "VMI26"'),
                  (len(d) > 5 and d[5] == 1, 'version != 1'),
                  (int.from_bytes(d[6:8], 'big') == 16, 'tamanio de memoria (bytes 6-7) != 16'),
                  (len(d) == esperado, 'tamanio de archivo %d != %d' % (len(d), esperado))]
        malos = [m for c, m in checks if not c]
        ok = not malos; motivo = '; '.join(malos)
    registrar('T07.c', 'Formato del .vmi (header VMI26 v1, 16 KiB, 16552 bytes)', ok, motivo)
    t = vmx(B + '.vmx', 'T07.vmi', stdin='g\nq\n', etq=B + '_d_quit')
    chequear('T07.d', "'q' en la 2da pausa aborta (no imprime Paso 3)", t,
             contiene=['Paso 1', 'Paso 2'], no_contiene=['Paso 3'])
    t = vmx('T07.vmi', stdin='g\ng\n', etq=B + '_e_reanudar')
    chequear('T07.e', 'Reanudar desde T07.vmi solo -> imprime Paso 3 y chequeos OK', t,
             todo_ok=True, contiene=['Paso 3'], no_contiene=['Paso 1'])
    t = vmx(B + '.vmx', 'T07_m32.vmi', 'm=32', stdin='g\ng\n', etq=B + '_f_m32')
    ok = os.path.isfile('T07_m32.vmi') and os.path.getsize('T07_m32.vmi') == 8 + 128 + 32 + 32 * 1024
    registrar('T07.f', 'Con m=32 el .vmi pesa 8+128+32+32768 bytes', ok,
              'tamanio = %s' % (os.path.getsize('T07_m32.vmi') if os.path.isfile('T07_m32.vmi') else 'no existe'))
    t = vmx(B + '.vmx', 'T07.vmi', stdin='\ng\ng\n', etq=B + '_g_paso')
    chequear('T07.g', 'Enter = paso a paso (y luego g) termina bien', t, todo_ok=True, contiene=['Paso 3'])

errores = [('T08a', 'T08a_overflow_push', 'PUSH sin espacio -> STACK OVERFLOW', r'overflow', ['Primer PUSH OK']),
           ('T08b', 'T08b_overflow_call', 'CALL recursivo -> STACK OVERFLOW', r'overflow', []),
           ('T09a', 'T09a_underflow_pop', 'POP con pila vacia -> STACK UNDERFLOW', r'underflow', ['3 POP OK']),
           ('T09b', 'T09b_underflow_ret', 'RET con pila vacia -> STACK UNDERFLOW', r'underflow', ['3 POP OK'])]
for id_, base, desc, rx, cont in errores:
    if quiere(id_) and traducir(base):
        t = vmx(base + '.vmx', etq=base)
        chequear(id_, desc, t, regex=rx, contiene=cont, no_contiene=['NO DEBERIA VERSE'])

if quiere('T10') and traducir('T10_memoria'):
    for m, debe_fallar in (('m=1', True), ('m=2', True), (None, False), ('m=64', False)):
        args = ['T10_memoria.vmx'] + ([m] if m else [])
        t = vmx(*args, etq='T10_memoria_' + (m or 'default').replace('=', ''))
        if debe_fallar:
            chequear('T10.' + m, 'Con %s -> MEMORIA INSUFICIENTE' % m, t, regex=r'memoria|insuficiente',
                     no_contiene=['proceso cargado -> OK'])
        else:
            chequear('T10.' + (m or '16'), 'Con %s el proceso carga bien' % (m or '16 KiB por defecto'), t,
                     contiene=['proceso cargado -> OK'])

if quiere('T11') and traducir('T11_disassembler'):
    t = vmx('T11_disassembler.vmx', '-d', etq='T11_disassembler')
    chequear('T11', 'Disassembler: constantes, >, sectores y modificadores', t,
             contiene=['48 6F 6C 61 00', '"Hola"', '61 62 63 64 65 66 00', '"abcdef"',
                       '46 75 6E 64 61 6D ..', '"Fundamentos"', 'PUSH', 'POP', 'CALL', 'RET',
                       'b[DS', 'w[EDX', 'l[DS', 'w[ES', 'AH', 'EL', 'CX'])
    lineas_ep = [l for l in t.splitlines() if l.lstrip().startswith('>')]
    registrar('T11.ep', "Exactamente una linea marcada con '>' (entry point)", len(lineas_ep) == 1,
              'lineas con >: %d' % len(lineas_ep))

if quiere('T12') and traducir('T12_compat_v1', version=1):
    t = vmx('T12_compat_v1.vmx', etq='T12_compat_v1')
    chequear('T12', 'Programa de la Parte I (header v1) imprime 42', t, contiene=['42'])

print()
tot = len(resultados); ok = sum(1 for r in resultados if r[2])
print('=' * 60)
print('TOTAL: %d/%d OK' % (ok, tot))
if ok < tot:
    print('Fallaron:', ', '.join(r[0] for r in resultados if not r[2]))
print('Salidas completas en:', OUT)
sys.exit(0 if ok == tot else 1)
