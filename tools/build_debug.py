#!/usr/bin/env python3
"""Local debug APK build using the project's configured Gradle and SDK.

Uses only an existing SDK/JDK/Gradle and a writable per-project analysis cache.
This is a debug-development signing key, never a release key.
"""
from pathlib import Path
import os
import subprocess
import shutil
import re

PROJECT=Path(__file__).resolve().parents[1]
OUT=PROJECT.parent/'metalmax-analysis';OUT.mkdir(exist_ok=True)
SDK=Path(os.environ.get('ANDROID_HOME','/Users/bytedance/Library/Android/sdk'))
wrapper=(PROJECT/'gradle/wrapper/gradle-wrapper.properties').read_text()
version=re.search(r'gradle-([\d.]+)-(bin|all)\.zip',wrapper)
if not version:raise SystemExit('Cannot identify the configured Gradle distribution')
gradle_version,kind=version.groups();modern=int(gradle_version.split('.')[0])>=8
default_jdk='/Applications/Android Studio.app/Contents/jbr/Contents/Home' if modern else '/Users/bytedance/Library/Java/JavaVirtualMachines/jbr_dcevm-11.0.16/Contents/Home'
JDK=Path(os.environ.get('METALMAX_BUILD_JDK',default_jdk))
gradle=next((Path('/Users/bytedance/.gradle/wrapper/dists')/f'gradle-{gradle_version}-{kind}').glob(f'*/gradle-{gradle_version}/bin/gradle'))
cache=OUT/'gradle-home';cache.mkdir(exist_ok=True)
for name in ['modules-2','jars-9']:
    source=Path('/Users/bytedance/.gradle/caches')/name
    target=cache/'caches'/name
    if source.exists() and not target.exists():shutil.copytree(source,target)
if not modern:
    # Only the old local JDK11 lacks jlink. Modern AGP uses the complete JBR
    # and runs its normal JavaCompile tasks without replacing them.
    classes=PROJECT/'app/build/intermediates/javac/debug/classes';classes.mkdir(parents=True,exist_ok=True)
    sources=list((PROJECT/'app/src/main/java').rglob('*.java'))
    sources+=list((PROJECT/'app/build/generated/source/buildConfig/debug').rglob('*.java'))
    result=subprocess.run(['javac','--release','11','-encoding','UTF-8','-cp',str(SDK/'platforms/android-33/android.jar'),'-d',str(classes)]+list(map(str,sources)),capture_output=True,text=True)
    (OUT/'java-build.log').write_text(result.stdout+result.stderr)
    if result.returncode: raise SystemExit(result.returncode)
key=OUT/'debug.keystore'
if not key.exists():
    subprocess.run([str(JDK/'bin/keytool'),'-genkeypair','-keystore',str(key),'-storepass','android','-alias','androiddebugkey','-keypass','android','-keyalg','RSA','-keysize','2048','-validity','10000','-dname','CN=Android Debug,O=Android,C=US'],check=True)
init=OUT/'debug-signing.gradle'
init.write_text('beforeSettings { s -> s.pluginManagement.resolutionStrategy.eachPlugin { r -> if (r.requested.id.id in ["com.android.application", "com.android.library"]) { r.useModule("com.android.tools.build:gradle:" + r.requested.version) } } }\n'
               'allprojects { afterEvaluate { p -> if (p.name == "app") { p.android.signingConfigs.debug.storeFile = new File("'+str(key)+'") } } }\n')
env=os.environ.copy();env['ANDROID_HOME']=str(SDK);env['JAVA_HOME']=str(JDK)
command=[str(gradle),'--offline','--no-daemon','--gradle-user-home',str(OUT/'gradle-home'),'-I',str(init),'-p',str(PROJECT),':app:assembleDebug']
if not modern:command+=['-Pandroid.aapt2FromMavenOverride='+str(SDK/'build-tools/33.0.2/aapt2'),'-x',':app:compileDebugJavaWithJavac']
result=subprocess.run(command,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
(OUT/'apk-build.log').write_text(result.stdout);print(result.stdout[-4000:])
if result.returncode: raise SystemExit(result.returncode)
apk=PROJECT/'app/build/outputs/apk/debug/app-debug.apk'
shutil.copyfile(apk,OUT/'metalmax-debug.apk');print(OUT/'metalmax-debug.apk')
