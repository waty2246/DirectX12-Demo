ddsview.exe file.dds
texassemble -array -o tarray.dds t0.dds t1.dds t2.dds t2.dds
texconv -m 10 -f BC3_UNORM -y tarray.dds

Output : filename - DXGI_FORMAT[ArraySize].MipLevels=textureWidth x textureHeight