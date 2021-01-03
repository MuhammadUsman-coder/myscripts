docker pull ubuntu:bionic
docker pull cassandra:2.1
sudo sysctl net.ipv4.conf.all.forwarding=1
sudo iptables -P FORWARD ACCEPT

git clone https://github.com/OPENAIRINTERFACE/openair-epc-fed.git
cd openair-epc-fed
git checkout master
git pull origin master
./scripts/syncComponents.sh

docker build --target oai-hss --tag oai-hss:production --file component/oai-hss/docker/Dockerfile.ubuntu18.04  component/oai-hss
docker image prune --force
docker image ls

docker build --target oai-mme --tag oai-mme:production --file component/oai-mme/docker/Dockerfile.ubuntu18.04  component/oai-mme
docker image prune --force
docker image ls

docker build --target oai-spgwc --tag oai-spgwc:production --file component/oai-spgwc/docker/Dockerfile.ubuntu18.04 component/oai-spgwc
docker image prune --force
docker image ls

docker build --target oai-spgwu-tiny --tag oai-spgwu-tiny:production --file component/oai-spgwu-tiny/docker/Dockerfile.ubuntu18.04 component/oai-spgwu-tiny
docker image prune --force
docker image ls

#UE and eNB
sudo apt-get install linux-image-lowlatency linux-headers-lowlatency
sudo apt-get install i7z
sudo i7z

sudo apt-get install cpufrequtils
sudo vi /etc/default/cpufrequtils
GOVERNOR="performance"


sudo apt-get update
sudo apt-get install subversion git
echo -n | openssl s_client -showcerts -connect gitlab.eurecom.fr:443 2>/dev/null | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca-certificates.crt
git config --global http.sslverify false

git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git
cd openairinterface5g
source oaienv
cd cmake_targets/
./build_oai -I -w USRP --eNB --UE
