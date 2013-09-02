#!/bin/sh

#Version info:
architecture=64bit
#architecture=32bit
tagver=build109.$architecture
ver=4.0.0
fullver=$ver.$tagver
version=osx_$fullver

drodlocaldir=DROD_Dev

#Names of Data dir bundles used for different versions and builds.  Add more as needed.
datfile_demo=DemoData.$ver.rar
datfile_full=FullData.$ver.rar

#Name to give the .dmg (i.e. install bundle) as it's created
dmg_build_name=drod-$ver.dmg

#Final name for the .dmg (based on build type).  Add more options as needed.
#specialbuildname=-ArcadeTown
specialbuildname=

dmg_beta_download_name=drod-gateb-osx-beta-$fullver.dmg
dmg_demo_download_name=drod-gateb-osx$specialbuildname-demo-$fullver.dmg
dmg_full_download_name=drod-gateb-osx$specialbuildname-full-$fullver.dmg

#buildtype=release
buildtype=custom

#cat $0

#username=mrimer
#cd ~$username/Caravel
cd ~/Caravel

mv $drodlocaldir $drodlocaldir-$version
cd $drodlocaldir-$version
cd Master/Darwin
#make drod-beta
#chmod -R +w $buildtype
#make clean-$buildtype

#Alternate builds:
#For SDL_mixer sound lib:
#make VARIANT_FLAGS="-DMANIFESTO -DCARAVELBUILD -DUSE_SDL_MIXER" drod-$buildtype
#For fmod sound lib:
#make VARIANT_FLAGS="-DMANIFESTO -DCARAVELBUILD" drod-$buildtype

#########################################################################
#Types of builds to produce

#Beta demo version:
#rm ../../Data.rar
#ln -s $datfile_demo ../../Data.rar
#make demobundle-beta
#mv beta/bin/$dmg_build_name ../../../$dmg_beta_download_name

#GatEB Demo version:
rm ../../Data.rar
ln -s $datfile_demo ../../Data.rar
make demobundle-$buildtype
mv $buildtype/bin/$dmg_build_name ../../../$dmg_demo_download_name

#GatEB Full version:
rm ../../Data.rar
ln -s $datfile_full ../../Data.rar
make bundle-$buildtype
mv $buildtype/bin/$dmg_build_name ../../../$dmg_full_download_name

####################################################################

cd ~/Caravel
mv $drodlocaldir-$version $drodlocaldir
