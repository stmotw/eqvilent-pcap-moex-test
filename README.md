# eqvilent-pcap-moex-test
```
mkdir data
pushd data
wget http://ftp.moex.ru/pub/SIMBA/Spectra/prod/pcap/2021-11-10.1844-1910.zip
unzip 2021-11-10.1844-1910.zip
popd

cmake . && make

cat data/Corvil-13052-1636559040000000000-1636560600000000000.pcap | build/eqvilent > data/parsed.txt
```
