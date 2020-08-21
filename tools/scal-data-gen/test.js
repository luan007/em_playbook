const fetch = require('node-fetch');

var str = {"items":[{"content":"前不久为大家带来了 Flutter 1.20，这也是目前规模最大的更新，在运行流畅、界面美观、开发高效和保持开放性四个方面都进展颇多。现在是时候准备迈出下一步了，我们诚邀您参与本次☟Flutter 开发者问卷调研☟，来帮助 Flutter 做到更好。本次调研会较为详细地了解大家在移动和 Web 等平台上使用 Flutter 时各个方面的体验，并会着重了解大家对一些动画效果的看法。您可以随时离开问卷页面，并在方便的时候返回继续作答，但请注意本次调研将于 2020 年 8 月 27 日截止。此次调研问卷完全匿名，我们不会搜集您的个人信息，仅会从统计层面了解大家在开发实践、产品体验以及日常角色分","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652063299&sn=6c7be9b28c5375a5fcfeab1d48f873b3&idx=1","title":"Flutter 因你更优秀 | 第三季度开发者调研","desc":"2020/8/20&nbsp;&nbsp;&nbsp;#Flutter"},{"content":"一年一度的 Google Cloud Next '20: OnAir 再次吸引了全球云计算领域人士的 目光。大会聚焦云计算产业，分为九大主题为与会者提供丰富多彩的干货内容。为让中国用户汲取大会的精华，助力中国企业扬帆出海、智赢全球，我们从大会的相关内容中精选出中国客户比较关注、适合中国企业发展的部分，并邀请了多位 Google Cloud 资深行业专家，组织三期 “Google Cloud Next '20 精选课” 线上直播系列活动。9月10日-24日每周四15:00-16:00精选课程准时直播，敬请关注！ 第一周：9月10日 构建行业未来——深化游戏、媒体、零售、制造行业转型 15:00-","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652063298&sn=0a0335e37c862998447cb3c2c6b793c1&idx=1","title":"【直播预告】Google Cloud Next &#39;20 精选课开课啦！","desc":"2020/8/19&nbsp;&nbsp;&nbsp;#Google Cloud"},{"content":"在往期 #11WeeksOfAndroid 系列文章中我们介绍了联系人和身份、隐私和安全、Android 11 兼容性、开发语言、Jetpack、Android 开发者工具，本期将聚焦 Google Play 应用分发与盈利。我们将为大家陆续带来 #11WeeksOfAndroid 内容，深入探讨 Android 的各个关键技术点，您不会错过任何重要内容。本期的 11 Weeks of Android 聚焦 Google Play 上应用的分发与盈利。我们秉持初心，持续优化 Android 平台。Google Play 与开发者密切合作，为数十亿 Android 用户提供了惊人的线上体验。从一","link":"http://mp.weixin.qq.com/s?__biz=MzAwODY4OTk2Mg%3D%3D&mid=2652062890&sn=ab68fd4aeded57377a8dedbac6a4a8e6&idx=1","title":"聚焦 Android 11: Google Play 应用分发与盈利","desc":"2020/8/18&nbsp;&nbsp;&nbsp;#Google Play"}]}

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