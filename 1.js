var a = {
    "users": [
        {
            "id": "USER1",
            "name": "Horizon",
            "password": "asdoifjasdoifjalsdkfjalskdfjalskdjfl",
            "projects": [
                "UUUIDDSAFASDFSDAFDSA"
            ]
        }
    ],
    "projects": [
        {
            "id": "UUUIDDSAFASDFSDAFDSA",
            "meta": {
                "description": "百度展厅南京"
            }
        }
    ],
    "assets": [
        {
            "project": "UUUIDDSAFASDFSDAFDSA",
            "id": "uUIDFU98U312498U23OIRHJSDLKJFNAKSMDFNKLJASDNF",
            "type": "advancedMovie",
            "data": {
                "url": "http://oss.cache.aliyun.com/3.mp4",
                "trim": {
                    "start": 0,
                    "end": 0.1
                },
                "volume": {
                    "mute": true
                }
            }
        },
        {
            "project": "UUUIDDSAFASDFSDAFDSA",
            "id": "asdfkjasdfhkjasdfdlkasjdflkasd",
            "type": "kvpair",
            "data": {
                "asfd": "asdfasfdfa"
            }
        },
        {
            "project": "UUUIDDSAFASDFSDAFDSA",
            "id": "asdlfkjaslkdfj",
            "type": "HORZ_CUSTOM_1",
            "data": {
                "asfd": "asdfasfdfa"
            }
        }
    ],
    "experience_types": {
        "smartCity_v2": {
            "name": "智慧城市展示模组2.0",
            "meta": {
                "bg": {
                    "descName": "背景",
                    "desc": "呈现大屏幕大视频或大图 建议分辨率>xxxx",
                    "type": [
                        "video",
                        "advancedVideo",
                        "image"
                    ]
                },
                "yaml": {
                    "descName": "数据文件",
                    "desc": "智慧城市YAML数据定义",
                    "type": [
                        "smartcity_custom_yaml"
                    ]
                },
                "title": {
                    "descName": "展项标题",
                    "desc": "用于欢迎的文字",
                    "type": [
                        "string"
                    ]
                }
            }
        }
    },
    "experiences": [
        {
            "project": "UUUIDDSAFASDFSDAFDSA",
            "id": "uUIDFU98U312498U23OIRHJSDLKJFNAKSMDFNKLJASDNF",
            "type": "smartCity_v2",
            "data": {
                "bg": {
                    "type": "asset",
                    "display": "大背景3",
                    "link": "uUIDFU98U312498U23OIRHJSDLKJFNAKSMDFNKLJASDNF"
                },
                "yaml": {
                    "type": "asset",
                    "display": "yaml文件上传2",
                    "link": "uUIDFU98U312498U23OIRHJSDLKJFNAKSMDFNKLJASDNF"
                },
                "title": {
                    "type": "asset",
                    "link": "{数据集/title}"
                }
            }
        }
    ]
}

// /edit/*
// /exhibit/*


// POST /edit/{proj}/{asset}
// GET /edit/{proj}/{asset}



// GET /exhibit/{USB_ID}?key=xxxx
// ao.loadData("smartCity_v2", USB_ID, {
//     bg: ,
//     title: ,
//     yaml: 
// }).update((d)=>{
//     d = {
//         bg: {},
//         title: {},
//         yaml: {}
//     };
// });



var shared = {
    state: {
        page: define_slider(5, {min: 1, max : 7, name : '展厅亮度控制'}),
        scene : 2,
        ease_open : 0.5
    }
}


register(shared, 'io');
