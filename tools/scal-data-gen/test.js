const fetch = require('node-fetch');

var str = {"items":[{"content":"欢迎使用 Jetpack DataStore，这是一个经过改进的全新数据存储解决方案，旨在替代原有的 SharedPreferences。Jetpack DataStore 基于 Kotlin 协程和 Flow 开发，并提供两种不同的实现: Proto DataStore 和 Preferences DataStore。其中 Proto DataStore，可以存储带有类型的对象 (使用 protocol buffers 实现)；Preferences DataStore，可以存储键值对。在 DataStore 中，数据以异步的、一致的、事务性的方式进行存储，克服了 SharedPrefere","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652076039&sn=49259c54924dde908c64b6bde6ff57af&idx=1","title":"使用 Jetpack DataStore 进行数据存储","desc":"2021/1/8&nbsp;&nbsp;&nbsp;#Android"},{"content":"点击上方图片，即刻收听！ 新年伊始，万象更新。倏忽之间，Google Play 开发者播客节目 Apps  Games & Insights 第二季的 8 期内容已经全部与您见面，不同领域的开发者通过多种视角与主题，探讨海外市场开发与发行的经验心得。海外开发者在新常态下如何应对机遇和挑战？又如何借助 Google 工具打造高质量应用？您可以在本季节目中寻找答案。我们特意整理了 Apps  Games & Insights 第二季播客节目合辑，希望这些分享能为开发者带来启发，帮助您更有信心地迎接崭新的 2021！ 注: 节目回顾 09 新常态下，海外线上教育平台的应对之道 关键词:新常态、教育类","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652075959&sn=c5479e492b65074177b95e78c842853e&idx=1","title":"精彩回顾 | Apps, Games & Insights 播客节目合辑","desc":"2021/1/7&nbsp;&nbsp;&nbsp;#Google Play"},{"content":"人们更倾向于安装并保留较小和安装占用空间更小的应用，在新兴市场中尤为明显。有了 R8 编译器，您可以通过压缩、混淆和优化，更全面的缩小应用体积。本文我们将对 R8 的特性进行一个简要的介绍，并介绍可预期的代码缩减程度以及如何在 R8 中启用这些功能。R8 的压缩特性 R8 通过下面 4 项特性来减少 Android 应用大小: 为什么需要 R8 压缩 开发应用时，所有代码都应有目的并在应用中实现相应功能。不过，大多数应用都会使用 Jetpack、OkHttp、Guava、Gson 和 Google Play 服务等第三方库，并且用 Kotlin 编写的应用始终包含 Kotlin 标准库。当您使","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652075647&sn=a6b4c50e0e9c30b3e290dbba3e6b8eca&idx=1","title":"使用 R8 压缩您的应用","desc":"2021/1/6&nbsp;&nbsp;&nbsp;#Android"}]}

fetch('http://emerge.ltd:10011/upload', {
    method: 'post',
    body:    JSON.stringify(str),
    headers: { 'Content-Type': 'application/json' },
})



var test = `
MoveTo 516, 746 微信
Delay 1168
LeftClick 1
Random 0, 0
Random 0, 0
Random 0, 0
Random 0, 0
MoveTo 285, 225 人头按钮
Delay 2000
LeftClick 1
Random 0, 0
Random 0, 0
Random 0, 0
Random 0, 0
MoveTo 421, 300 公众号列表
Delay 1000
LeftClick 1
Random 0, 0
Random 0, 0
Random 0, 0
Random 0, 0
MoveTo 759, 179 公众号
Delay 1777
LeftClick 1
Random 0, 0
Random 0, 0
Random 0, 0
Random 0, 0
MoveTo 960, 418 历史消息
Delay 1000
LeftClick 1
MoveTo 444, 59 
Delay 200
MoveTo 445, 54 刷新
Delay 3452
MoveTo 447, 100 标题
LeftClick 1
MoveTo 447, 355
Delay 3047
PageDown 1
Delay 681
PageDown 1
Delay 681
PageDown 1
Delay 681
PageDown 1
Delay 681
LeftDown 1
Delay 6
Random 0, 0
Random 0, 0
Random 0, 0
Random 0, 0
MoveTo 474, 359
Delay 816
MoveTo 978, 17
Delay 1602
LeftClick 1
MoveTo 1096, 83
Delay 1884
LeftClick 1
`