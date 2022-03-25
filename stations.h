#ifndef STATION_H
#define STATION_H

#define NUM_STATIONS 68

struct station_t {
    char callsign[7];
    char URL[100];
};

struct station_t stations[NUM_STATIONS] = {
{
    "CFBU",
    "http://s2.stationplaylist.com:8110/;&lang=sp&title=CFBU.ca&lang=en"
},
{
    "CFCR",
    "http://68.183.199.246:8000/m4a"
},
{
    "CFEP",
    "http://cassini.shoutca.st:8215/stream"
},
{
    "CFFF",
    "http://trentradio.ca:8800/hi-fi"
},
{
    "CFIS",
    "https://ais-sa1.streamon.fm/7157_64k.aac"
},
{
    "CFMH",
    "http://cfmh-listen.unbsj.ca:8000/"
},
{
    "CFML",
    "https://ais-sa1.streamon.fm/7161_64k.aac"
},
{
    "CFMU",
    "https://stream.cfmu.ca/mount.128mp3"
},
{
    "CFPT",
    "https://ice10.securenetsystems.net/CFPT"
},
{
    "CFRC",
    "https://audio.cfrc.ca/stream"
},
{
    "CFRO",
    "http://listen-coopradio.sharp-stream.com/coopradio.mp3"
},
{
    "CFRU",
    "https://archive.cfru.ca/cfruhi2"    
},
{
    "CFUR",
    "http://listen.cfur.ca/"
},
{
    "CFUV",
    "https://ais-sa1.streamon.fm/7132_64k.aac"
},
{
    "CFUZ",
    "http://192.99.35.215:5073/peachcityradio"
},
{
    "CFWN",
    "http://northumberland.serverroom.net:8850/"    
},
{
    "CFYT",
    "http://stream.cfyt.ca:8000/stream"    
},
{
    "CHBB",
    "https://c11.radioboss.fm:18134/stream"    
},
{
    "CHCR",
    "http://158.69.227.214:8066/live"
},
{
    "CHES",
    "https://streaming.radio.co/saa34f11f4/listen"
},
{
    "CHET",
    "http://209.53.182.2:8000/stream.mp3"    
},
{
    "CHHA",
    "https://stream.statsradio.com:8062/stream"    
},
{
    "CHLY",
    "http://184.69.114.194:8000/stream.mp3"
},
{
    "CHMA",
    "https://securestreams6.autopo.st:2111/"
},
{
    "CHMR",
    "http://mp3srv.munsu.mun.ca:8001/"
},
{
    "CHOD",
    "https://stream2.statsradio.com:8032/live"
},
{
    "CHRW",
    "https://949fm.ca/stream"    
},
{
    "CHRY",
    "https://ice7.securenetsystems.net/CHRY"    
},
{
    "CHSR",
    "https://usa6.fastcast4u.com/proxy/chsrbroa?mp=%2Fstream"
},
{
    "CHUO",
    "https://stream.statsradio.com:8102/stream"
},
{
    "CHXL",
    "http://69.11.9.241:88/broadwave.mp3?src=1&rate=0&ref=&src=1&rate=0"
},
{
    "CHYZ",
    "http://ecoutez.chyz.ca:8000/mp3"    
},
{
    "CICK",
    "https://usa13.fastcast4u.com/proxy/smithers?mp=%2F1"    
},
{
    "CILU",
    "http://luradio-server.lakeheadu.ca:8000/stereo128.mp3"    
},
{
    "CITR",
    "https://live.citr.ca/live.aac"
},
{
    "CIVL",
    "https://live.civl.ca:8000/live.mp3"    
},
{
    "CIWS",
    "https://audio-edge-kef8b.ams.s.radiomast.io/5fcb1172-3cfc-48e2-92d8-8ff86401c1bd"    
},
{
    "CJAM",
    "http://stream.cjam.ca:8000/"
},
{
    "CJBU",
    "http://142.12.32.15:8000/stream.m3u"
},
{
    "CJLO",
    "http://rosetta.shoutca.st:8883/stream"
},
{
    "CJLX",
    "https://lweb04.lcaat.ca:8443/91x.mp3"    
},
{
    "CJLY",
    "https://relay1.cjly.net/64"    
},
{
    "CJMP",
    "http://192.99.4.210:3264/stream"    
},
{
    "CJNU",
    "http://noasrv.caster.fm:10188/backup"    
},
{
    "CJQC",
    "http://us3.streamingpulse.com:8074/;stream.mp3"    
},
{
    "CJRU",
    "https://ryerson.out.airtime.pro/ryerson_a"    
},
{
    "CJSF",
    "https://www.cjsf.ca:8443/listen"    
},
{
    "CJSR",
    "http://ais-sa1.streamon.fm/7093_24k.aac"
},
{
    "CJSW",
    "https://cjsw.leanstream.co/CJSWFM"    
},
{
    "CJTR",
    "http://108.178.13.122:8152/;"    
},
{
    "CJUC",
    "http://stream.openbroadcaster.com:8033/CJUC"    
},
{
    "CJUM",
    "https://live.umfm.com:8010/;stream/1"    
},
{
    "CKAR",
    "https://thebay887.ca:8002/;stream.mp3"
},
{
    "CKCU",
    "https://stream2.statsradio.com:8124/stream"    
},
{
    "CKDU",
    "https://archive1.ckdu.ca:9750/ckdu_1_on_air_low.mp3"    
},
{
    "CKHA",
    "https://24493.live.streamtheworld.com/CKHAFMAAC_SC"    
},
{
    "CKMS",
    "http://stream2.statsradio.com:8024/stream"    
},
{
    "CKOA",
    "https://s45.myradiostream.com/:15204/;"    
},
{
    "CKTZ",
    "https://s5.voscast.com:8219/stream.mp3?_=1"    
},
{
    "CKUW",
    "http://shout.mtl.gameservers.com:9025/;"    
},
{
    "CKUA",
    "http://ais-sa1.streamon.fm/7000_64k.mp3"    
},
{
    "CKVE",
    "https://us3.streamingpulse.com/ssl/CoveFM"    
},
{
    "CKVS",
    "http://s2.stationplaylist.com:9240/listen.aac"
},
{
    "CKXU",
    "https://stream.ckxu.com/stream/2/"
},
{
    "VF2590",
    "https://audio-edge-vqwx4.yyz.g.radiomast.io/fa7ff97e-4b53-4cc8-939c-43aadc971a4c?t=dark" 
},
{
    "CJSE",
    "https://stream2.statsradio.com:8008/stream"
},
{
    "CIXX",
    "https://ice23.securenetsystems.net/CIXXFM"
},
{
    "CIUT",
    "https://ice23.securenetsystems.net/CIUT"    
}
};

#endif