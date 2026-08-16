我现在的fps地图，存在高层环廊和登上环廊的斜坡和1层地面，未来可能还有些障碍物，怎么做AI追踪啊，如果要无视视野/距离一直追

你现在的 AI 是平面朝玩家 XY 直冲，无视视野已经做到了；但环廊在 z=3、地面在 z=0，直冲会撞墙底/卡斜坡，所以问题变成「怎么走到玩家那一层」。

一直追（无视野）
目标永远是玩家：target = player，不要 sightRadius / LOS。参考工程那套 AIGetClosestEnemyInsight 可以直接丢掉。

多层怎么追
方案	适不适合你这图
继续直冲 XY
同层还行；跨层会废
瓦片 HeatMap（参考工程 2D 地牢）
你地图不是 tile，且缺高度连通
手搓路点图
最合适：地面 / 斜坡 / 环廊几个点连起来
完整 NavMesh
24h 作业过重
推荐：路点图 + 永远追玩家
在地图里放少量 NavNode（或 XML 里写死坐标）：
地面几个点
斜坡两端（上坡口、下坡口）——跨层必经
环廊四边中点
边：能直线走到且不穿墙的就连（环廊沿走廊连；地面↔环廊只通过斜坡两端）。
每帧（或隔几帧）：
找离 AI 最近的节点 A、离玩家最近的节点 B
图上最短路 A→…→B
朝路径下一个路点走（到了再换下一个）
若 AI 与玩家已同层且直线可达，可短路：直接朝玩家走。
这样「无视距离一直追」= 永远以玩家为终点重算路径；障碍物以后加节点/断边即可。

更糙的临时方案（只有一条斜坡时）
若 |玩家z - AI.z| 大 或 直线被挡:
    先走向斜坡入口（地面→上，环廊→下）
否则:
    直冲玩家
一条斜坡够用，以后障碍多了还是换路点图。

结论： 追踪目标用玩家全局坐标；移动用带斜坡连通的路点/导航图，不要用纯 XY 追。参考工程的视野追和 2D HeatMap 都不直接搬你这张 3D 环廊图。
navmesh有多重

对你这张 15×15 + 一条斜坡 + 环廊 的作业图来说，完整 NavMesh 偏重，不是算法课难度，是工程量。

大概要扛什么
块	工作量
从 AABB/OBB 几何生成可行走面（或手画 mesh）
大
高度连通（斜坡当 walkable link）
中大
查询：点落在哪块 poly、到目标最短路径
中
Agent 沿路径拐弯、卡边、掉下环廊
中
障碍物动态更新（可选）
再加一截
成熟方案是 Recast/Detour 一类；自己从零做「能用的」也往往要几天调试，和 24h take-home 不对等。

和路点比
NavMesh：通用、以后地图复杂也稳，但搭建+调参沉没成本高。
十几个路点：半天能追能上坡，评分叙事也够讲清楚。
一句话： NavMesh 不是「算一下就完」，是一套系统；你这规模用路点图更划算，真要写进简历再考虑简化版（手绘几块 walkable poly + A*）也比全自动生成轻。

能细致讲讲吗，因为我是ai辅助的所以压力不会特别大

可以。下面按「它是什么 → 怎么建 → 怎么用 → 你这张图怎么落 → AI 辅助时怎么拆活」讲细一点。

1. NavMesh 在解决什么
寻路要回答两件事：

哪儿能走（可行走区域）
从 A 怎么走到 B（在可行走区域里找一条路）
路点图：人工把「能走的关键位置」收成点，边表示「两点之间能直走」。
NavMesh：把可行走区域收成 多边形拼起来的地面网，角色在多边形内部任意点都能走，路径更贴几何。

对你：地面整块、环廊一圈、斜坡一块斜面——理论上三层 walkable surface，用 mesh 连起来。

2. 一张 NavMesh 长什么样
概念上：

┌──────────── 地面 poly 若干 ────────────┐
│                                        │
│              斜坡 poly（倾斜）          │── portal 连到
│                                        │     环廊 poly
└────────────────────────────────────────┘
Polygon：一块可走凸多边形（三角形也行）
邻居边 / Portal：两块相邻 poly 的共享边，寻路时从这块跨到那块
Agent 参数：半径、能否上台阶、最大坡度——决定 mesh 要往内缩多少（否则贴墙会卡）
查询时：

把「AI 脚底」「玩家脚底」投影/落点到某个 poly 上
在 poly 图上做 A*（节点是 poly，或更细的点）
得到 poly 序列后，用 漏斗算法（string pulling） 拉成平滑折线
AI 朝折线下一个拐弯点走
比路点多出来的就是：落点、邻接图、漏斗平滑；不是「换个 A*」那么简单。

3. Mesh 从哪来（三条路，工作量差很多）
A. 手绘 / 手写（最轻，很适合你）
直接在数据里定义几块 walkable：

Floor：z≈0 的大矩形（或减去柱子洞）
Gallery：环廊顶面几块矩形（可拆成 4 条）
Ramp：斜坡表面一个四边形（用 OBB 顶面四个角）
再声明邻接：

地面 ↔ 斜坡低端
斜坡高端 ↔ 南环廊
AI 辅助很好写：XML/代码里列顶点 + 邻居索引。没有 Recast，没有体素化。
代价：地图改形状要手改 mesh；障碍物多了要切洞、加 poly。

B. 从碰撞几何自动切（中等）
读 AABB/OBB 的「顶面」当候选 walkable，按高度分层（0 和 3），斜坡单独识别，自动算邻接。
难点：环廊「内侧悬空」别生成可走；墙顶不要当路；斜坡法线/连接容差容易出洞。
调试时间往往比手绘多。

C. Recast/Detour 一类（最重）
输入三角化世界 → 体素 → 区域 → poly mesh → Detour 查询。
能力强，但集成、参数（cell size、agent height、climb）、导出、和你自定义物理对齐，是一整条管线。作业里除非你明确想秀集成，否则性价比一般。

你压力不大的话：优先 A；想多练再碰 B；C 当加分项。

4. 运行时每帧（或每 N 帧）做什么
Update AI:
  start = ProjectToNavMesh(ai.feet)
  goal  = ProjectToNavMesh(player.feet)
  if 失败 → 待机 / 往最近有效点走
  if 冷却到了 or 目标 poly 变了:
     path = FindPath(start, goal)   // poly A* + string pull
  steer toward path[corner]
  到拐弯点 → 下一个
注意：

不必每帧全量 A*：玩家小范围动就复用路径，或 0.2s 重算一次。
一直追、无视视野：goal 永远是玩家落点，不要 LOS gate。
物理仍是你的胶囊：NavMesh 只给「想去的方向/下一点」，别用 mesh 替代碰撞，否则斜坡对不齐会滑/陷。
5. 落到你当前地图（具体该切几块）
结合你现在的 TestMap：

区域	建议
地面 15×15 @ z=0
1 块大 poly（或中间留空若以后有柱）
环廊顶 @ z=3
东/西/南/北（北还可拆桥）几块矩形
斜坡 OBB 顶面
1 块四边形，两端分别连地面与南廊
跨层只通过斜坡两端 portal，否则 AI 会「直线穿地」以为能从地面飞到廊上。

障碍物以后：

静态箱子：从地面 poly 挖洞或拆成多块
或障碍只挡物理、不改 mesh（AI 会往箱子里挤）——短作业可以，观感差就改 mesh
6. 和「路点图」差在哪（帮你选型）
路点	手绘 NavMesh（A）
数据量
十几个点
几十个顶点 + 邻接
走位
贴点走，可能看起来绕
面内任意，更自然
实现点
A* on graph
落点 + poly A* + 漏斗
改地图
挪点
改多边形
AI 辅助
极顺
也顺，代码稍长
你说压力不大：手绘 NavMesh 完全可行，而且比集成 Recast 现实得多；若只想「能追能上坡」，路点仍更短。

7. AI 辅助时建议怎么拆任务（少返工）
按垂直切片，每片可跑：

数据结构：NavPoly（顶点、邻居、可选 layer/z）+ 从 XML 加载
落点：给定脚底 XYZ，找包含该 XY、高度接近的 poly（环廊/地面用 z 容差）
可视化：画出 poly 边和当前路径（调试神器，一定要先做）
A* on poly + 简单「中心点连中心」路径（先不漏斗也能追）
漏斗平滑（可选，观感升级）
AIController：目标=玩家，跟路径走；去掉视线判定
障碍：再谈挖洞
卡关时优先查：落点错层、斜坡没连上、agent 半径没内缩贴墙。

8. 和参考工程的关系
参考工程是 2D tile + HeatMap，默认平地连通；注释掉的寻路也不是 3D NavMesh。
你的环廊+斜坡是 2.5D，直接搬 tile 距离场会对不上高度；应单独做 walkable 图（路点或手绘 mesh），不要幻想「开个 NavMesh 开关」。

务实建议：
有 AI 辅助、想做得像样 → 做 手绘 walkable poly（轻量 NavMesh），斜坡当唯一跨层桥。
只求玩法闭环 → 路点 +「先去坡再追人」更快。

两者都是「图上寻路 + 一直以玩家为终点」；差别主要在表示可行走区域的精度，不是「会不会一直追」。

像我这种地图，丢到ue引擎里面直接一个巨大的navmesh罩住整个地图就能用了，我们能做成这样吗

不能指望「罩一个巨大盒子 = UE 那样就能用」。 UE 看起来一键，是因为底下已经帮你把几何体素化、找出可走表面、按高度分层并连好了；不是真的在空中铺了一张无视碰撞的大毯子。

UE 实际在干什么
你拖一个 Recast 体积罩住关卡之后，大致是：

读场景静态碰撞/网格
按 Agent 身高、半径、可爬高度体素化
得到贴着地面/斜坡/楼板的 walkable 表面（地面一层、环廊一层，中间空气不是 nav）
斜坡若坡度合法，会自动连上下层
所以观感是「罩一下就行」，产物仍是贴地的多层 mesh，不是一个实心巨盒。

你们现在若「罩一个大 AABB」
若只做「整个 arena 一个大矩形可走」：

地面追人：好像能用
环廊下穿过：路径会穿空气/穿实心廊体
上环廊：没有「必须走斜坡」的连通，AI 会直着「飞」向高处目标
多层 + 斜坡正是巨罩失效的地方。

那还能不能「做成 UE 那种手感」
可以接近，但省事的是工具，不是省略生成：

做法	像不像 UE 一键	你们成本
集成 Recast/Detour（或类似），体积罩住 → 从你们的 AABB/OBB 碰撞 bake
最像
集成 + 参数 + 导出，工程最大
自己写简易 bake：收集顶面/斜面 → 生成 poly → 邻接
半自动
中等，AI 辅助可行
手绘几块 walkable（地面/坡/廊）
不像一键，但结果同类
最轻，够用
真·一个大盒子
像，但不能用
—
结论
目标可以一样：最终也是一张（多层）NavMesh，AI 只跟路径走。
做不到的是：在自定义引擎里不 bake、只罩体积就自动正确——除非接上和 UE 同级的生成器。
有 AI 辅助时，更现实的是：手绘/半自动 bake 出「贴地的那几层」，而不是模拟 UE 的体积控件本身。
若你特别想要「改地图 → 按一下重生 Nav」的工作流，下一步应是 bake 管线（从 m_solidBounds / m_solidObbs 生成），而不是一个巨大可走 AABB。

怎么可能会飞，navmesh罩住整个地图，walkable仍旧是地面，而不是路上的空气

你说得对，是我前面把两件事混在一起了。

UE 里那个「大罩子」是什么
那是 生成范围（bounds），不是可走体积。
Bake 之后 walkable 仍然是：射线/体素落到的地面、楼板、合法斜坡表面。罩子里的空气本来就不会变成 nav。

所以 UE：体积罩全图 ≠ 在空气里走路。多层时一般是地面一层 mesh + 环廊一层 mesh，斜坡若合格会连上。

我之前说的「会飞」指的是另一种错误
在自己引擎、又没有 bake时，如果把「罩住全图的大盒子」直接当成 整块可走 poly（内部任意点都合法），路径就会穿空气——那是实现错了，不是 UE 模型。

正确类比 UE 应该是：

Bounds（可很大）
    → bake/采样 → 贴在碰撞顶面上的 walkable
    → AI 只在这些表面上寻路
对你们的含义
目标可以做成 UE 那种：罩范围随便大，walkable 仍只在地面/廊顶/斜坡上。
难点不在罩子，在 bake：从你们的 AABB/OBB 顶面（和斜坡面）生成那层表面；没有这一步，只画一个大底面矩形，AI 不会飞，但会 上不了环廊（上层根本不在 mesh 上）。
之前表述不严谨，谢谢纠正。

我打算给AI解释，帮我完成“我们尝试做一个navmesh系统，设置navmesh区块后读取AABB/OBB顶面当候选walkable，”

先对一下你们地图里 AABB/OBB 的数据结构，再写成可直接丢给实现 AI 的说明。

下面是一段可直接复制给实现 AI 的说明（按你们现有 Map / AABB·OBB 碰撞来写）。你可按需改项目名或路径。

给实现 AI 的任务说明（复制用）
目标： 在自定义引擎 FPS 项目里做一版轻量 NavMesh：先配置 NavMesh 区块（生成范围）→ bake 时读取该范围内固体 AABB/OBB 的顶面作为候选 walkable → 生成可寻路面，供 AI 无视视野/距离追玩家。

背景与约束
地图是手写几何：Map 里已有 m_solidBounds（AABB3）和 m_solidObbs（OBB3），来自 MapDefinitions.xml 的 <AABB> / <OBB>（含斜坡 rampStart/rampEnd）。
不是 UE Recast 全集成；不是把 NavMesh 体积内部填成可走空气。
NavMesh 区块 = bake bounds（生成范围）；walkable = 落在碰撞顶面上的表面。
地图有：一层地面（约 z=0）、高层环廊（顶约 z=3）、OBB 斜坡连通上下；以后可能有障碍物。
AI 要 始终知道玩家位置并追（不做 sight/LOS gate）。现有 AIController 是 XY 直冲，跨层会坏，需改为跟 Nav 路径走。
先做垂直切片，不要一次做完 Recast/动态障碍/疏导。
第 1 切片：区块 + 从顶面 bake 候选 walkable（本次重点）
NavMesh 区块配置（二选一或都支持，优先简单）：

在 MapDefinition XML 增加例如：
<NavMeshVolume name="Arena" mins="-8,-8,-1" maxs="8,8,7"/>
或代码/常量一个罩住整张 TestMap 的 AABB。
语义：只处理与该 volume 相交 的固体；volume 本身不是 walkable。
Bake 输入： Map 在 BuildGeometry() 之后的 m_solidBounds / m_solidObbs。

候选 walkable 规则：

AABB： 取 顶面（z = maxs.z 的矩形四顶点）。若顶面中心（或整体）不在 NavMesh volume 内则跳过。
OBB： 取 +K 方向那一面（与渲染 FACE_TOP 一致的那侧）四个角点，作为可能倾斜的四边形（斜坡靠这个）。
过滤（务必做，避免墙顶/高墙当路）：
可选：忽略厚度过大且顶面过高的「墙」（例如顶面 z > 某阈值且 AABB 在 XY 很瘦）——先用简单启发式，或 XML 给几何加 walkable="true/false"，环廊顶/地板/斜坡标 true，外墙标 false。
更稳的 v1： 给 map 几何加可选属性 navWalkable="true"，只 bake 标记为 true 的；Floor、Gallery*、Ramp 打开，Wall* 关闭。
Agent 半径可先不做 shrink（v1）；若易贴墙卡住，再对 poly 内缩。
输出数据结构（建议）：

NavPoly: id, std::vector<Vec3> verts (凸四边形即可), neighbors[]
NavMesh: polys, bakeFrom(Map), FindPoly(position), DebugRender()
每个顶面先变成 一个凸 poly（矩形/四边形）。
邻接： 若两边共享接近共线的边（端点距离 < epsilon），且高度可过渡（或共享边在斜坡连接处），则互为 neighbor。斜坡高端应连到南环廊顶，低端连到地面。
邻接算不准时可 手写 link 作 fallback，但优先自动边匹配。
调试： 用 DebugRender 画出每个 walkable poly 边（可用不同颜色区分地面/环廊/斜坡）。没有可视化不要做寻路。

触发时机： Map::BuildGeometry() 末尾或 Startup 里 m_navMesh.Bake(*this)。

明确不要做（本切片）
不要把 volume 内部体素/实心当可走。
不要集成 Recast/Detour。
不要一上来做漏斗算法；下一片再做。
不要改物理碰撞语义；Nav 只读固体几何。
后续切片（本次可只留接口/TODO）
ProjectPoint：脚底位置 → 所在 poly（XY 在 poly 内且脚高度接近 poly 高度/平面）。
Poly 图 A* + 中心点路径。
AIController：goal=玩家；定期重寻路；朝路径点走；去掉视线限制。
再考虑 string pulling、agent 半径内缩、动态障碍。
验收（本切片）
加载 TestMap 后能看到：地面顶、环廊顶、斜坡顶被画成 walkable；墙顶尽量没有。
NavMesh volume 只影响「谁参与 bake」，空气不可走。
斜坡 poly 是倾斜的，不是水平贴在 z=0。
实现风格
贴合现有 Map / MapDefinition 风格；少造引擎层框架。
先可读可调，再优化。
改 XML 时注明要给哪些几何加 navWalkable。

第一切片做完了，我们继续

第一切片 indict：可以过，继续。

已有且质量不错：NavMeshVolume、navWalkable、AABB 顶面 / OBB +K 面 bake、邻接（含斜坡压在地板上的 DoesEdgeLieOnPoly）、FindPoly、按 kind 上色 + 邻接线 Debug。BuildGeometry 末尾 bake，Render 里画 mesh。

注意：Arena 和 Cave 是两张独立 NavMesh，跨 volume 不会自动连边；下一片寻路要在 Map 层统一查询，或 bake 后补跨 mesh 邻接（洞穴口附近）。

给实现 AI 的 Prompt（第 2 切片：寻路）
目标： 在现有 NavMesh bake（第 1 片）之上实现 poly 图 A* + 中心点路径，并提供 Map 级查询 API；本片先不要改 AI 追人逻辑（或只加可选 debug：按键把路径画到玩家），保证路径可视化可验收。

现状（不要推倒重来）
NavMesh / NavPoly：BakeFrom、BuildAdjacency、FindPoly、DebugRender 已可用。
Map 持有 std::vector<NavMesh> m_navMeshes（TestMap：Arena + Cave 两个 volume）。
AIController 仍是简单前进，本片可不接或只接 debug。
要做的事
路径结果结构（示例）：

struct NavPath {
  std::vector<Vec3> m_waypoints; // 世界坐标，通常为 poly 中心序列（含终点可投影）
  bool m_isValid = false;
};
NavMesh::FindPath(Vec3 start, Vec3 goal) const -> NavPath

startPoly = FindPoly(start)，goalPoly = FindPoly(goal)；任一失败 → invalid。
同 poly：waypoints = { goal 投影点或 goal }（或 start→goal）。
异 poly：在 m_neighbors 图上 A*（代价用 poly 中心距离即可）。
输出：从 startPoly 到 goalPoly 的中心点序列；建议首点用 start 在 startPoly 上的投影、末点用 goal 在 goalPoly 上的投影，中间用各 poly GetCenter()（可跳过与起终点同 poly 的重复中心）。
本片不做漏斗 / string pulling。
Map 级 API（重要：多 volume）

Map::FindNavPath(Vec3 start, Vec3 goal) const
策略（选一种写清并实现）：
推荐： 先在每个 NavMesh 上分别 FindPoly；若 start/goal 在同一 mesh → 用该 mesh FindPath。
若在不同 mesh：尝试在两 mesh 的 poly 间找「边接近/中心接近」的临时桥接边，合成一次路径；或 bake 后重建一张 合并图（所有 poly id 全局唯一 + 跨 volume 邻接）。
若暂时做不好跨 Cave↔Arena：至少保证 Arena 内（地面↔斜坡↔环廊）能通路，并在注释/Debug 里标明跨 volume 限制；优先把 Arena 通路做对。
投影辅助（可选小函数）
ProjectOntoPoly(poly, position) -> Vec3：沿 poly 法线投到平面并夹在凸包内（或夹失败则用 center）。FindPath 起终点用它。

调试

保留现有 poly/邻接 DebugRender。
增加：当存在「当前调试路径」时，用醒目颜色画 waypoint 折线（例如橙/白）。
可用临时方式生成调试路径：每帧 FindNavPath(某个 AI 或固定点, 玩家位置)，或控制台/按键触发一次。
启动或 bake 后可用 DebugAddMessage 打印一次「Arena 内从地板中心到环廊某点是否找得到路」。
验收

从地面一点到环廊一点：路径应经斜坡相关 poly，而不是穿墙直连。
同层两点：路径合理、邻接可达。
FindPoly 失败时路径 invalid，不崩。
Debug 能看见 waypoint 折线。
不要做
不接 Recast；不改 bake 顶面规则（除非发现邻接明显断了再修 epsilon）。
不做 string pulling、动态障碍、agent 半径 shrink。
不要大改 AIController 追击（留给第 3 片）；最多 debug 画路径。
风格
贴合现有 NavMesh.cpp / Map；A* 可放在 NavMesh.cpp 匿名命名空间。
保持可调：edge epsilon 等已有常量可复用，勿魔法数散落。
第 3 片会是：AIController 用 Map::FindNavPath、周期性重寻路、朝 waypoint 走、无视视野一直追。要我也可以把第 3 片草稿先附上，但建议先把第 2 片跑通验收。

你这个做法，我打个比方，如果怪物目前在斜坡侧面底下，想要追逐环廊上方的玩家，那他会直线朝向斜坡给定的poly点，视觉上会出现“怪物在顶墙”，擦着斜坡侧面的实心墙前进，而正确的应该不应该是怪顺着墙走，走到斜坡底端拐弯上坡吗

你的担心对，按 poly 中心点走时，很容易出现「擦着斜坡侧墙往坡中心钻」，而不是先绕到坡底再上坡。

为什么会这样
图上连通的是「地板 poly ↔ 斜坡 poly」（共享底边或边压在地板上），A* 只保证 拓扑正确（最终能上环廊）。

但若 waypoint 是：

当前位置（坡侧面旁的地板）→ 斜坡 poly 中心 → 环廊…
直线指向斜坡中心，会穿过/贴着 OBB 侧面实心，视觉上就像顶墙。
正确意图应是：先走到两块 poly 的 portal（共享边中点/可走入口），再沿斜坡表面走。

所以问题不在「会不会选斜坡」，而在 中心点路径忽略了入口在哪。

怎样才算走对
做法	效果
只用 poly 中心（第 2 片草案）
能通，拐弯丑，易贴侧墙
连续两 poly 用共享边 portal 当拐点
先被吸到坡底入口再上坡，多数情况就对了
再加漏斗 string pulling
更贴边、更短、更自然
物理沿墙滑
减轻穿透，但不会自己学会绕到坡底
对你这个例子：怪在坡侧面底下追廊上玩家 → 路径应是 绕到斜坡低端 portal → 沿坡上 → 环廊，而不是朝坡中心斜插。

和切片怎么衔接
第 2 片若只做中心点，验收「能通」可以，但你说的观感 bug 会在。
更值得在第 2 片直接要求：

相邻 waypoint：优先用共享边中点（portal），不要用下一 poly 中心当唯一目标；
起点/终点仍投影到所在 poly；
漏斗可留第 3 片。
如果你愿意，我可以把上一份 prompt 改成「portal 拐点版」再给你一版完整可复制文本。

UE里是如何实现的

UE（底层主要是 Recast 生成 + Detour 查询）不会拿「下一块 poly 的中心」当最终走路目标，所以你说的那种「朝斜坡中心顶侧墙」在默认流程里会好很多。

它实际分两步
1. 找走廊（corridor）
在 NavMesh 多边形图上做 A*（或类似），得到一串 相连的 poly：
地板… → 斜坡面 → 环廊…
这一步只保证「拓扑能通」，和我们第 2 片类似。

2. 拉直路径（string pulling / 漏斗算法）
在走廊里，用相邻 poly 之间的 portal（共享边） 当「门」，从起点到终点拉一条在走廊内尽可能直的折线。
Agent 跟的是这条折线上的拐点，不是每个 poly 的中心。

因此从「坡侧面旁的地板」去「廊上」：

走廊仍会经过斜坡那块 可走表面；
拉直后，拐点会落在 地板↔斜坡的入口 portal 附近，再沿坡面上去；
不会以「斜坡中心」为磁力点斜着插进 OBB 侧面。
和几何的关系
Bake 时：侧面实心不是 walkable，只有顶面/坡面进 mesh。
障碍会按 Agent 半径 内缩，路径天然离墙一条缝。
大地板常被切成很多小 poly，绕过坡底入口更自然；不是「一整块地板中心 ↔ 一整块坡中心」。
和当前草案的差别
中心点路径（我们草案）	UE / Detour
A*
poly 序列
poly 序列（走廊）
最终路径
poly 中心折线
portal 漏斗拉直
侧墙问题
容易朝坡心顶墙
沿入口进坡面
一句话： UE 也是先找 poly 走廊，但最后用 portal + string pulling 生成走路点；要接近那种手感，下一片应做 portal 拐点，有余力再加漏斗，而不是停在 poly 中心。

具体解释一下这个portal拐点做法

Portal 是什么
两块相邻 NavPoly 若能走过去，中间有一条 共享边（或一段重叠边）。这条边就是 portal（门）：离开 A、进入 B 时，脚必须从这扇门过（在 mesh 语义上）。

例：地板 ↔ 斜坡，portal ≈ 斜坡底边那条线；斜坡 ↔ 南环廊，portal ≈ 斜坡顶边。

和「中心点」差在哪
A* 先得到走廊（poly 序列），例如：

P0(地板) → P1(斜坡) → P2(南廊) → P3(东廊) → …
中心点做法：
start → center(P1) → center(P2) → … → goal
从坡侧面旁的地板直奔 center(斜坡) → 顶侧墙。

Portal 拐点做法：
在相邻两块之间插入 共享边的中点（或边上某点）：

start（投在 P0）
  → portal(P0↔P1)   // 坡底入口
  → portal(P1↔P2)   // 坡顶接廊
  → portal(P2↔P3)   // …
  → goal（投在最后一块）
怪会先被吸到 坡底门，再沿坡面到 坡顶门，再进环廊——不会以坡心为磁铁。

算法步骤（实现级）
A* 得到 poly id 列表：[i0, i1, …, ik]
waypoints.clear()
waypoints.push( Project(start, poly[i0]) )
对每个相邻对 (ia, ib)：
算共享边：两边 DoEdgesMatch / 部分重叠那条（你们 bake 邻接时用过的逻辑）
取边上一点，v1 常用 中点 mid = (edgeA + edgeB) * 0.5
若边很长，中点仍在门上，一般够用
waypoints.push(mid)
waypoints.push( Project(goal, poly[ik]) )
（可选）去掉几乎重合的连续点，避免抖
AI 每帧朝 当前 waypoint 走，靠近（如距离 < 0.3）则切下一个。

共享边怎么求
对邻居 polyA、polyB：

枚举 A 的每条边、B 的每条边
用现有的 DoEdgesMatch（或「边落在对方 poly 上」）找到匹配边
portal 段 = 重叠部分；简化版直接用 匹配到的那条边的中点
斜坡底边压在大地板内部、不是地板外框时：邻接可能靠 DoesEdgeLieOnPoly；此时 portal 就是 斜坡那条底边本身（整段都在地板 poly 上），中点仍可用。

和 UE 漏斗的关系
Portal 中点	String pulling（UE）
用什么
每扇门一个固定点（中点）
整条走廊上左右 portal 端点做漏斗
路径
折线，略绕、但进对门
走廊内尽量直
实现量
小
更大一点
Portal 中点已经能修掉「顶斜坡侧墙」；漏斗是在同一走廊上把折线再拉直。

一张示意图（俯视）
      环廊
  ============
       ↑ portal 顶
      /坡/
     /  /
    /  /  ← 不要朝坡中心斜插
   /__/
    ↑ portal 底（正确拐点）
----+-----------  地板
    ↑
 怪在坡侧面旁：先走到 portal 底，再上坡
一句话： 走廊仍由 A* 定；走路点改成 「下一扇门」而不是「下一块地的中心」。

