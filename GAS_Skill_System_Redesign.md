# GAS 技能系统重构方案

## 1. 设计目标

这套方案的目标不是重写 GAS，而是在 GAS 上方增加一层清晰的技能编排层。

GAS 继续负责最终权威结算：

- Attribute 修改
- GameplayEffect 应用
- GameplayTag 授予和移除
- Buff duration 和 stack
- GameplayCue 复制
- Cost 和 Cooldown
- 网络同步和预测基础

新的技能系统负责回答这些问题：

- 玩家输入应该激活哪个技能
- 技能如何释放
- 技能释放出来的载体如何存在和命中
- 命中或持续触发时应该结算哪些效果
- Buff 如何作为持续状态影响后续技能
- 镜头和输入上下文如何随技能进入和退出

核心原则：

```text
Ability 负责身份和流程
Delivery 负责命中载体
Payload 负责触发点到目标
EffectConfig 表达结算意图
AbilityEffectExecutor 翻译成 GAS 操作
GameplayEffect 负责最终权威结算
```

## 2. 元判断标准

整个设计里最重要的判断标准是：

```text
这个差异，是应该定义一个新类型，还是某个已有结构上可以打开或关闭的能力？
```

选错会导致组合爆炸。比如：

```text
ChargedBranched
ComboBranched
ChannelBranched
```

选对后，结构会更薄：

- 条件分支演出不是新的 CastFlow 类型，而是 `CastFlowConfig.BranchRules`
- 生命周期归属不是焊死在 Delivery 类型里，而是 `DeliveryConfig.LifecyclePolicy`
- 施法保护 Tag 和收尾清理不是某个 GA 子类专属逻辑，而是配置驱动 Ability 的通用能力
- 镜头变化不是 GameplayEffect，而是 CameraMode 系统的持续控制状态
- 输入重映射不是 RequiredTags，而是 InputContext 系统

## 3. 命名规则

统一命名规则：

```text
Definition：独立资产，可复用
Config：嵌在 Definition 里的配置块
Runtime：运行时执行体或执行状态
Handle：运行时清理凭证
Executor：统一执行服务
```

第一阶段建议使用的核心类型：

```text
AbilityDefinition
  CastFlowConfig
  DeliveryConfigs[]
    PayloadConfigs[]
      EffectConfigs[]

BuffDefinition
InputContextDefinition
CameraModeDefinition

AbilityEffectExecutor
GameplayEffect
```

命名边界：

- `AbilityDefinition` 是技能主资产
- `BuffDefinition` 是 Buff 统一数据入口
- `InputContextDefinition` 是输入上下文资产
- `CameraModeDefinition` 是镜头模式资产
- `CastFlowConfig` 是 AbilityDefinition 内嵌的释放流程配置
- `DeliveryConfig` 是 AbilityDefinition 内嵌的释放物配置
- `PayloadConfig` 是 DeliveryConfig 内嵌的触发配置
- `EffectConfig` 是 PayloadConfig 内嵌的结算意图

如果未来某个 Delivery 需要大量复用，例如 Fireball Projectile、SwordWave Projectile、WindEye AoE，可以再把 `DeliveryConfig` 提升为独立 `DeliveryDefinition` 资产。

## 4. 整体架构

四层骨架：

```text
1. 输入解析层
   SkillSlotConfig / InputContextDefinition
   负责 Slot + Gesture + InputContext -> AbilityDefinition

2. 施法流程层
   CastFlowConfig
   负责技能怎么演、什么时候触发 Delivery

3. 释放物层
   DeliveryConfig
   负责释放物怎么活、怎么命中、生命周期归谁管

4. 效果结算层
   PayloadConfig / EffectConfig / AbilityEffectExecutor
   负责触发之后对谁结算什么，并翻译成 GAS 操作
```

真正让 CastFlow、Delivery、Effect 解耦的关键，是第 4 层必须是唯一入口。

无论触发来自：

- 角色自己的近战 Trace
- Projectile 命中
- AoE Tick
- Beam Tick
- Summon 攻击
- Buff 周期触发

最终都应该组装成统一的 `AbilityEffectContext`，交给 `AbilityEffectExecutor` 处理。

## 5. AbilityDefinition

`AbilityDefinition` 是技能主资产，是现有 `UAbilityDefinitionConfig` 方向的升级版。

职责：

```text
描述技能是什么，以及它由哪些配置组成。
```

建议字段：

```text
AbilityID
DisplayName
Icon
AbilityClass

RequiredTags
BlockedTags
ActiveStateTag

CostConfig
CooldownConfig

CastFlowConfig
DeliveryConfigs[]
```

它不负责：

- 当前命中了谁
- 当前连招第几段
- 当前蓄力百分比
- 当前 Projectile 是否爆炸
- 当前 AoE Tick 到第几次

这些都是运行时状态，应该存在 GameplayAbility、CastFlow runtime、Delivery runtime 或 AbilityEffectContext 里。

## 6. SkillSlotConfig 和输入解析

`SkillSlotConfig` 描述职业固定技能槽位的默认绑定。

示例：

```text
SkillSlotConfig
  Slot: Primary / Secondary / Shift / E / Q / Space
  Gesture: Tap / HoldStart / HoldRelease / Repeat
  InputContext: Default
  AbilityDefinition: Ability.TwinSword.LightCombo
```

Slot 是固定的，但 Slot 不等于技能。

例如 TwinSword：

```text
Primary Tap       -> TwinSword.LightCombo
Primary Hold      -> TwinSword.HeavyAttack
Secondary Tap     -> TwinSword.SecondaryCombo
Space Tap         -> Dodge
```

例如 Mage：

```text
RMB HoldStart     -> Mage.SpellWeaving
Q Tap Default     -> Mage.NormalQ
Q Tap SpellWeave  -> Mage.LightningArc
```

输入解析回答的是：

```text
这个按键现在应该解释成哪个技能或行为？
```

AbilityDefinition 的 RequiredTags 和 BlockedTags 回答的是：

```text
解释出来的技能现在能不能释放？
```

这两个问题不要混在一起。

## 7. InputContextDefinition

`InputContextDefinition` 是临时输入重映射表。

它不替代 Enhanced Input。

Enhanced Input 负责物理输入：

- 按下
- 松开
- 长按阈值
- Tap
- Hold
- Triggered

InputContextDefinition 负责技能语义解析：

```text
当前状态下，Slot + Gesture 代表哪个技能或行为？
```

建议字段：

```text
Bindings:
  Slot
  Gesture
  ActionType:
    Ability
    NativeCommand
    Passthrough
    Blocked
  AbilityDefinition
```

示例：法师组合施法状态

```text
InputContext.Mage.SpellWeaving
  LMB Tap -> Mage.FireBolt
  RMB Tap -> Mage.IceShard
  Q Tap   -> Mage.LightningArc
  E Tap   -> Mage.HealRune
  Space   -> Mage.Finisher
  Shift   -> Passthrough
```

示例：弓箭手瞄准状态

```text
InputContext.Archer.Aim
  LMB Tap     -> Archer.AimedShot
  LMB Release -> Archer.ChargedShot
  E Tap       -> Archer.MarkTarget
  Space       -> Dodge 或 Blocked
```

## 8. InputContextHandle

`InputContextHandle` 是运行时清理凭证，不是玩法概念。

用途：

```text
PushInputContext(Context) -> InputContextHandle
RemoveInputContext(Handle)
```

不要简单 Pop 栈顶，因为会误删别的系统压入的上下文。

问题示例：

```text
SpellWeaving Push MageSpellContext
AimMode Push AimContext
SpellWeaving End -> PopInputContext()
```

如果此时栈顶是 AimContext，SpellWeaving 就会错误移除别人的上下文。

正确做法：

```text
SpellWeavingHandle = PushInputContext(MageSpellContext)
AimHandle = PushInputContext(AimContext)

SpellWeaving End:
  RemoveInputContext(SpellWeavingHandle)
```

它和 GAS 的 `FActiveGameplayEffectHandle` 是同一类思路：

```text
谁 Push，谁用自己的 Handle Remove
```

## 9. CastFlowConfig

`CastFlowConfig` 描述技能如何释放。

它只管：

- 技能怎么演
- 如何播放 Montage
- 什么时候 Commit
- 什么时候触发 Delivery
- 什么时候允许输入接续
- 什么时候结束
- 什么时候取消

它不管：

- Projectile 怎么飞
- AoE 怎么查目标
- 伤害怎么结算
- Buff 怎么应用

建议字段：

```text
Type:
  Instant
  Charged
  Combo
  Channel
  Aim
  Stance

Montages
TriggerPointActions
BranchRules
CommitPolicy
CancelPolicy
```

常见类型：

```text
Instant
  按下后立即释放。
  例：普通技能、瞬发法术、翻滚。

Charged
  按住蓄力，松开释放。
  例：重击、弓箭蓄力、蓄力火球。

Combo
  连续输入推进段数。
  例：左键三连、轻重派生。

Channel
  按住持续释放，松开或被打断结束。
  例：持续闪电、站桩回蓝。

Aim
  进入瞄准状态，释放时发射。
  例：弓箭右键瞄准。

Stance
  切换到临时输入上下文。
  例：法师右键进入组合施法状态。
```

## 10. TriggerPointActionConfig

`TriggerPointActionConfig` 描述固定流程节点到达时一定会做的动作。

它和 BranchRule 的区别：

```text
TriggerPointAction：固定节点动作
BranchRule：满足条件才改变流程
```

常见 TriggerPoint：

```text
OnStart
OnHoldThresholdReached
OnCommit
OnRelease
OnCancel
OnInterrupt
OnEnd
```

常见 Action：

```text
PlayMontage
SpawnDelivery
ApplyPayload
ApplyBuff
PushInputContext
PushCameraMode
RemoveInputContext
RemoveCameraMode
```

示例：法师组合施法进入状态

```text
CastFlowConfig: Stance
  OnHoldThresholdReached:
    PlayMontage: SpellWeaving_Start
    ApplyBuff: Buff.Mage.SpellWeavingActive
    PushInputContext: InputContext.Mage.SpellWeaving
    PushCameraMode: CameraMode.Mage.SpellWeaving

  OnRelease / OnInterrupt / OnCancel:
    RemoveInputContext
    RemoveCameraMode
    RemoveOwnedBuffs
```

## 11. BranchRuleConfig

`BranchRuleConfig` 描述条件分支。

`Branched` 不单列为 CastFlow 类型，因为分支是所有 CastFlow 都可能拥有的能力。

建议第一阶段收窄，只支持明确场景：

```text
OnInput -> JumpToSection
OnEvent -> AddDelivery
OnHit -> ApplyPayload
OnTagAdded -> JumpToSection
```

暂时不做：

```text
ReplaceDelivery
复杂嵌套 Conditions
通用脚本式 Result 列表
PushInputContext in BranchRule
多层 BranchRule 链式执行
```

避免把 DataAsset 做成小型脚本语言。

示例：完美闪避

```text
Ability.Dodge
  CastFlow: Instant
  BranchRule:
    Trigger: OnEvent PerfectDodgeSuccess
    Conditions:
      TimeWindow: PerfectDodgeWindow
    Result:
      PlayMontage: PerfectDodgeMontage
      ApplyPayload:
        Effects:
          ApplyBuff Buff.PerfectDodge
          GameplayCue Cue.PerfectDodge
```

示例：完美弹反

```text
Ability.Guard
  CastFlow: Channel 或 Stance
  BranchRule:
    Trigger: OnHitReceived
    Conditions:
      TimeWindow: PerfectParryWindow
    Result:
      PlayMontage: PerfectParryCounter
      AddDelivery: ForwardSlashMelee
```

`ForwardSlashMelee` 自己通过 Payload OnHit 进入统一 Effect 管线。

## 12. DeliveryConfig

`DeliveryConfig` 描述技能载体。

它回答：

```text
技能释放出来的东西怎么存在、怎么命中、生命周期归谁管？
```

建议字段：

```text
Type:
  Melee
  Projectile
  AoE
  Beam
  Summon
  Self

RuntimeClass
LifecyclePolicy:
  BoundToAbility
  Detached
  Persistent

PayloadConfigs[]
```

Delivery 不应该负责伤害和 Buff。

它只负责：

- Start
- Stop
- Trace
- Movement
- Collision
- Tick
- 触发 Payload

命中之后交给 Payload 和 EffectExecutor。

## 13. Delivery 类型

建议类型：

```text
Melee
  角色自身武器轨迹或范围判定。
  例：双刀普通攻击。
  通常不需要 Actor。

Projectile
  飞出去的实体。
  例：火球、箭矢、十字剑气。
  通常是 Actor。

AoE
  区域效果。
  例：雷电领域、风眼、火圈。
  可以是 Actor，也可以是 AbilityTask。

Beam
  射线、链接、持续指向效果。
  例：闪电束、激光。
  可以是 Component、Actor 或 Runtime。

Summon
  生成独立单位。
  例：召唤物、炮台。
  通常是 Actor。

Self
  不生成外部载体，直接对自己触发 Payload。
  例：回血、强化状态、进入隐身。
```

不要强行让所有 Delivery 都是 Actor。

近战普通攻击不应该 Spawn Delivery Actor，它只需要：

```text
Montage NotifyState 打开 Trace Window
Trace 命中
Payload OnHit
Executor Apply Damage
```

## 14. LifecyclePolicy

`LifecyclePolicy` 不属于某个 Delivery 类型，而是 DeliveryConfig 上的独立配置。

建议三类：

```text
BoundToAbility
  寄生于 Ability。
  Ability 结束，Delivery 也结束。
  例：持续闪电、Beam、瞄准状态。

Detached
  放出去后独立存在，到自己生命周期结束。
  例：火球、剑气、风眼。

Persistent
  长期存在，有自己的行为或 AI。
  例：召唤物、炮台。
```

同样是 AoE：

- 持续雷电 AoE 是 BoundToAbility
- 风眼 AoE 是 Detached

所以生命周期不能写死在 AoE 类型里。

## 15. DeliveryRuntime

不要第一阶段做复杂大一统 DeliveryRuntime 基类。

最多定义一个很薄的接口：

```text
Start
Stop
```

不同类型自己实现：

```text
Melee
  AbilityTask 或 Ability 内部 Trace Runtime。

Projectile
  AActor。

AoE
  Actor 或 AbilityTask。

Beam
  Component / Runtime / Actor。

Self
  通常不需要 Runtime，直接触发 Payload。
```

第一阶段只需要 Melee 简版跑通。

## 16. PayloadConfig

`PayloadConfig` 是 Delivery 和 Effect 之间的桥。

它回答：

```text
Delivery 触发某个事件后，对谁执行哪些效果？
```

建议字段：

```text
Trigger:
  OnSpawn
  OnHit
  OnTick
  OnExpire
  OnEnter
  OnExit

TargetPolicy:
  Self
  Owner
  HitTarget
  AreaTargets

RequiredOwnerTags
RequiredTargetTags

EffectConfigs[]
```

示例：火球

```text
DeliveryConfig.FireballProjectile
  Payload OnSpawn:
    Effects:
      GameplayCue Cue.Fireball.Cast

  Payload OnHit:
    TargetPolicy: HitTarget
    Effects:
      Damage 30
      ApplyBuff Buff.Burn
      GameplayCue Cue.Fireball.Hit

  Payload OnExpire:
    Effects:
      SpawnDelivery FireballExplosionAoE
```

示例：雷电 AoE

```text
DeliveryConfig.LightningAoE
  Payload OnTick:
    TargetPolicy: AreaTargets
    Effects:
      Damage 8
      GameplayCue Cue.Lightning.Tick
```

## 17. EffectConfig

`EffectConfig` 表达结算意图。

它不是 GameplayEffect 的替代品。

边界：

```text
EffectConfig = 自己的配置层
GameplayEffect = GAS 的真实结算层
```

建议类型：

```text
Damage
Heal
Knockback
ApplyBuff
SpawnDelivery
GameplayCue
```

建议字段：

```text
Type
Magnitude
Duration
GameplayEffectClass
SetByCallerTag
BuffDefinition
CueTag
DeliveryConfig
```

Damage 示例：

```text
EffectConfig
  Type: Damage
  Magnitude: 30
  GameplayEffectClass: GE_InstantDamage
  SetByCallerTag: Data.Damage
```

执行时：

```text
MakeOutgoingSpec(GE_InstantDamage)
SetByCaller(Data.Damage, -Magnitude)
ApplyGameplayEffectSpecToTarget
```

不要直接：

```text
TargetHealth -= Damage
```

否则会绕开 GAS 的 Attribute、Modifier、Tag、Stack、Replication 和 Prediction。

## 18. AbilityEffectContext

`AbilityEffectContext` 是统一 Effect 管线的输入上下文，是整套系统的地基。

建议字段：

```text
SourceASC
SourceActor
TargetActors
TargetASCs
AbilityDefinition
DeliveryConfig
PayloadConfig
Trigger
HitResult
Location
Normal
AbilityLevel
Instigator
Causer
```

任何触发源都必须先组装成这个 context：

```text
Melee 命中
Projectile 命中
AoE Tick
Beam Tick
Buff 周期触发
SpawnDelivery 链式触发
```

然后交给 `AbilityEffectExecutor`。

## 19. AbilityEffectExecutor

`AbilityEffectExecutor` 是统一执行入口。

职责：

```text
检查 RequiredOwnerTags
检查 RequiredTargetTags
解析 TargetPolicy
执行 Damage
执行 Heal
执行 Knockback
ApplyBuff -> BuffDefinition
SpawnDelivery
ExecuteGameplayCue
```

它是唯一允许把 `EffectConfig` 翻译成 GAS 操作的地方。

不要让：

- Melee
- Projectile
- AoE
- Beam
- Summon

各自实现一套伤害和 Buff 逻辑。

## 20. BuffDefinition

第一阶段推荐采用“BuffDefinition 包装 GameplayEffect”的方案。

职责：

```text
BuffDefinition = Buff 的统一配置和展示入口
GameplayEffect = Buff 的真实 GAS 结算
```

建议字段：

```text
BuffID
DisplayName
Icon
Description

GameplayEffectClass
Duration
StackLimit
GrantedTags

bShowOnHUD
bCanDispel
```

执行方式：

```text
EffectConfig: ApplyBuff
  BuffDefinition: Buff.PerfectDodge

AbilityEffectExecutor:
  创建 BuffDefinition.GameplayEffectClass 的 Spec
  设置 Duration / SetByCaller
  添加 DynamicGrantedTags
  ApplyGameplayEffectSpecToTarget
```

第一阶段不做：

- AbilityModifiers
- 复杂属性公式
- Buff 之间互相触发
- 运行时动态拼复杂 AttributeModifier
- 复杂驱散规则

技能增强先用 Tag 条件解决。

示例：

```text
Buff.PerfectDodge
  GameplayEffectClass: GE_GenericDurationBuff
  Duration: 5
  GrantedTags:
    State.TwinSword.PerfectDodge

Combo Final Hit Payload
  RequiredOwnerTags: State.TwinSword.PerfectDodge
  Effects:
    Damage 额外伤害
    GameplayCue Cue.PerfectDodgeAttack
```

## 21. CameraModeDefinition

镜头状态不要放进 GameplayEffect。

镜头变化分两类：

```text
瞬时镜头反馈
  走 GameplayCue。
  例：命中震屏、完美闪避瞬间镜头冲击、大招 camera shake。

持续镜头模式
  走 CameraModeDefinition。
  例：弓箭瞄准、法师组合施法、格挡镜头、锁定镜头。
```

建议字段：

```text
FOV
SpringArmLength
CameraOffset
BlendInTime
BlendOutTime
Priority
bUseAimOffset
bLockOnTarget
```

运行时：

```text
PushCameraMode(CameraMode) -> CameraModeHandle
RemoveCameraMode(Handle)
```

它和 InputContextHandle 是同一类思路。

进入瞄准状态：

```text
PushInputContext Archer.Aim
PushCameraMode Archer.AimCamera
ApplyBuff State.Archer.Aiming
```

退出时统一清理。

## 22. CameraModeHandle

`CameraModeHandle` 是运行时清理凭证。

用途：

```text
CameraModeHandle = PushCameraMode(CameraModeDefinition)
RemoveCameraMode(CameraModeHandle)
```

必须在这些情况下清理：

- Ability End
- Ability Cancel
- Ability Interrupt
- 角色死亡
- 角色切职业
- 控制权丢失

所有 `PushCameraMode` 和 `PushInputContext` 最好由 GameplayAbility 记录 Handle，然后在 `EndAbility()` 统一清理。

## 23. Cost 和 Cooldown

Cost 和 Cooldown 仍然应该走 GAS 原生 CommitAbility。

AbilityDefinition 可以有：

```text
CostConfig
CooldownConfig
```

但执行时不要手动：

```text
if Stamina < Cost return
Stamina -= Cost
```

推荐：

```text
AbilityDefinition.CostConfig
  -> 选择或生成 Cost GameplayEffect
  -> CommitAbility
  -> GAS 应用 Cost GE
```

原则：

```text
配置层可以自定义
执行层仍然用 GAS CommitAbility
```

这样冷却、消耗、Tag Block 和网络同步会更稳。

## 24. 运行流程

完整运行流程：

```text
Enhanced Input 产生 Slot + Gesture
  -> InputContext / SkillSlotConfig 解析 AbilityDefinition
  -> GAS 检查 RequiredTags / BlockedTags / Cost / Cooldown
  -> CastFlowConfig 执行释放流程
  -> TriggerPointAction 或 BranchRule 启动 Delivery
  -> Delivery 触发 Payload
  -> Payload 生成 AbilityEffectContext
  -> AbilityEffectExecutor 执行 EffectConfig
  -> GameplayEffect 完成最终属性、Tag、Buff、复制
  -> GameplayCue / CameraMode 负责表现
```

## 25. LightCombo 实现方案

LightCombo 是第一阶段最适合验证系统的技能。

它需要：

```text
AbilityDefinition + Combo CastFlow + Melee Delivery + OnHit Payload + Damage Effect
```

### 25.1 AbilityDefinition

```text
AbilityDefinition.TwinSword.LightCombo
  AbilityID: Ability.TwinSword.LightCombo
  AbilityClass: GA_ComboMontage
  RequiredTags: 可选
  BlockedTags: State.Dead / State.Stunned / State.CastingHeavy
  ActiveStateTag: State.TwinSword.LightCombo.Active

  CostConfig:
    Stamina: 首次消耗，第一阶段不要做每段消耗

  CastFlowConfig:
    Type: Combo

  DeliveryConfigs:
    ComboHit_1
    ComboHit_2
    ComboHit_3
```

### 25.2 CastFlowConfig

```text
CastFlowConfig
  Type: Combo
  Montage: AM_TwinSword_LightCombo

  Sections:
    [0]
      SectionName: Light_01
      PlayRate: 1.0
      ChainInputWindowEvent: Event.Combo.ChainWindow
      ActiveDelivery: ComboHit_1

    [1]
      SectionName: Light_02
      PlayRate: 1.0
      ChainInputWindowEvent: Event.Combo.ChainWindow
      ActiveDelivery: ComboHit_2

    [2]
      SectionName: Light_03
      PlayRate: 1.0
      ActiveDelivery: ComboHit_3
```

CastFlow 只负责：

- 播放 section
- 监听输入缓存
- 在连招窗口推进 section
- 在该段攻击窗口启用对应 Delivery

它不负责伤害数值。

### 25.3 Montage Notify

每段动画里放武器检测窗口：

```text
Light_01
  NotifyState: WeaponTraceWindow Begin / End

Light_02
  NotifyState: WeaponTraceWindow Begin / End

Light_03
  NotifyState: WeaponTraceWindow Begin / End
```

Notify 不直接造成伤害，只通知当前 Melee Delivery：

```text
Trace Begin
Trace Tick
Trace End
```

### 25.4 DeliveryConfigs

第一段：

```text
DeliveryConfig.ComboHit_1
  Type: Melee
  LifecyclePolicy: BoundToAbility
  Payloads:
    OnHit:
      TargetPolicy: HitTarget
      Effects:
        Damage 10
        GameplayCue Cue.Hit.TwinSword.Light
```

第二段：

```text
DeliveryConfig.ComboHit_2
  Type: Melee
  LifecyclePolicy: BoundToAbility
  Payloads:
    OnHit:
      TargetPolicy: HitTarget
      Effects:
        Damage 15
        GameplayCue Cue.Hit.TwinSword.Light
```

第三段：

```text
DeliveryConfig.ComboHit_3
  Type: Melee
  LifecyclePolicy: BoundToAbility
  Payloads:
    OnHit:
      TargetPolicy: HitTarget
      Effects:
        Damage 25
        Knockback 300
        GameplayCue Cue.Hit.TwinSword.HeavyImpact
```

这样“不同段数不同伤害”不需要代码判断。

当前 section 启用了哪个 DeliveryConfig，就自然使用哪个 Payload 和 EffectConfig。

### 25.5 EffectConfig

Damage 走 GAS：

```text
EffectConfig Damage
  Type: Damage
  Magnitude: 10 / 15 / 25
  GameplayEffectClass: GE_InstantDamage
  SetByCallerTag: Data.Damage
```

Executor 执行：

```text
MakeOutgoingSpec(GE_InstantDamage)
SetByCaller(Data.Damage, -Magnitude)
ApplyGameplayEffectSpecToTarget
```

### 25.6 运行流程

```text
LMB Tap
  -> Input 解析到 AbilityDefinition.TwinSword.LightCombo
  -> GA_ComboMontage 激活
  -> CommitAbility
  -> Apply ActiveStateTag
  -> 播放 Light_01

Light_01 武器窗口打开
  -> Start Delivery ComboHit_1
  -> Trace 命中敌人
  -> Payload OnHit
  -> EffectExecutor 执行 Damage 10 + Cue

玩家在 ChainWindow 内再次按 LMB
  -> 输入缓存
  -> 跳 Light_02
  -> Start Delivery ComboHit_2
  -> 命中 Damage 15

第三段同理
  -> ComboHit_3
  -> Damage 25 + Knockback
```

## 26. 迁移和落地顺序

第一阶段只做最小闭环：

```text
1. FAbilityEffectContext
2. FEffectConfig
3. UAbilityEffectExecutor
4. FPayloadConfig
5. FDeliveryConfig 简版，只支持 Melee
6. 一个 GA 读取 AbilityDefinition 执行
7. UBuffDefinition 简版
```

第一个验证技能：

```text
TwinSword LightAttack
  Montage Notify 打开 Trace
  命中敌人
  Payload OnHit
  EffectExecutor 执行 Damage
  Apply GE_Damage
  Execute GameplayCue
```

第二个验证技能：

```text
TwinSword CrossSlash
  Montage 触发 Projectile
  Projectile OnHit
  复用同一套 Payload + EffectExecutor
```

## 27. 第一阶段明确暂不做

暂时不做：

```text
复杂 BranchRule 脚本系统
完整 DeliveryRuntime 大一统基类
Buff AbilityModifiers
复杂 InputContext 条件系统
复杂 CameraMode 优先级系统
链式 SpawnDelivery 大量配置
ReplaceDelivery
运行时动态拼复杂 AttributeModifier
```

先把最小闭环跑通，再按真实技能需求扩展。

## 28. 风险和约束

### 28.1 不要用 EffectConfig 重写 GameplayEffect

错误方向：

```text
EffectConfig 直接修改 Health
```

正确方向：

```text
EffectConfig 描述结算意图
Executor 创建 GameplayEffectSpec
GameplayEffect 完成最终结算
```

### 28.2 不要让 BranchRule 变成脚本系统

BranchRule 第一阶段只解决明确需求。

不要在 DataAsset 里做复杂逻辑编程。

### 28.3 不要为了统一强行 Actor 化

Projectile 可以是 Actor。

Melee 不应该为了统一而 Spawn Actor。

### 28.4 不要让 AbilityDefinition 保存运行时状态

Definition 是资产。

运行时状态必须放在 GameplayAbility、Runtime 或 Context 中。

### 28.5 不要让 InputContext 替代 Enhanced Input

Enhanced Input 负责物理输入。

InputContext 负责技能语义解析。

## 29. 总结

最终架构可以概括为：

```text
AbilityDefinition：技能数据入口
CastFlowConfig：释放流程
DeliveryConfig：命中载体
PayloadConfig：触发点到目标
EffectConfig：结算意图
AbilityEffectContext：统一结算上下文
AbilityEffectExecutor：翻译成 GAS 操作
GameplayEffect：最终权威结算
BuffDefinition：Buff 配置和展示入口
InputContextDefinition：输入语义重映射
CameraModeDefinition：持续镜头模式
```

这套系统的核心价值：

```text
技能流程、命中载体、效果结算、Buff、输入上下文、镜头状态互相解耦。
```

第一阶段最重要的是跑通：

```text
Melee 命中 -> Payload OnHit -> EffectExecutor -> GameplayEffect
```

这个闭环成立后，再逐步接入 Projectile、AoE、Buff、InputContext 和 CameraMode。

## 30. 实现约定与决策记录

这一节用来沉淀"一旦定下来就很难改、过段时间又容易忘记原因"的基础设施决策。后续每做一个类似的决定，都在这里追加一条。

### 30.1 GameplayTag 管理规则

结论：

```text
声明来源：DefaultGameplayTags.ini / Project Settings -> GameplayTags
C++ 角色：FRiftGameplayTags 作为访问缓存层，不做主声明
第一阶段：不使用 Native Tag
```

原因：

新系统里大量字段本质是配置数据：

```text
RequiredTags
BlockedTags
ActiveStateTag
GrantedTags
RequiredOwnerTags
RequiredTargetTags
CueTag
SetByCallerTag
```

随着内容增长，会频繁新增类似这样的标签：

```text
Ability.Mage.FireBolt
State.Mage.SpellWeaving.Active
Buff.TwinSword.PerfectDodge
Cue.Hit.TwinSword.Light
```

如果继续纯 C++ 原生注册，每加一个配置用的标签都要改代码、重新编译，和数据驱动的方向相悖。

边界划分：

```text
声明在 ini（Project Settings 面板编辑，ini 是落盘格式，不建议手改）
使用在 C++（FRiftGameplayTags 统一 RequestGameplayTag 并缓存）
```

概念示意：

```cpp
Data_Damage = RequestGameplayTag("Data.Damage");
State_Dead  = RequestGameplayTag("State.Dead");
```

只有极少数系统级、几乎永不由配置侧新增、且必须被 C++ 强依赖的标签，才考虑用 Native Tag。第一阶段建议完全不用，保持声明来源单一。

推荐 taxonomy：

```text
Ability.*
State.*
Buff.*
Event.*
Data.*
Cue.*
Cooldown.*
InputContext.*
Delivery.*
```

示例：

```text
Ability.TwinSword.LightCombo
Ability.TwinSword.HeavyAttack
Ability.Mage.SpellWeaving

State.Dead
State.Stunned
State.TwinSword.LightCombo.Active
State.Mage.SpellWeaving.Active
State.Archer.Aiming

Buff.TwinSword.PerfectDodge
Buff.Mage.ComboStep

Event.Combo.ChainWindow
Event.Montage.SpawnDelivery

Data.Damage
Data.Heal
Data.Duration
Data.StaminaCost
Data.ManaCost

Cue.Hit.TwinSword.Light
Cue.Ability.Mage.FireBolt.Hit
```

### 30.2 落实这条规则时要一并定下来的细节

这些是"从纯原生注册切换到字符串请求"之后才会出现的新坑，原来的纯 C++ 注册方式不会遇到，必须主动补偿。

**1. 初始化时机**

```text
RequestGameplayTag 必须在 GameplayTagsManager 完成 ini 加载之后调用
调用太早会静默返回 invalid tag，且不会报错
需要明确 FRiftGameplayTags 在哪个生命周期节点统一初始化并缓存
不要依赖"第一次被访问时惰性请求"
```

**2. 容错策略**

```text
纯原生注册时代：标签名拼错 = 编译失败，问题立刻暴露
切到字符串请求后：标签名拼错 = 运行时静默返回空 tag
建议：缓存层统一使用 ErrorIfNotFound = true
把"编译期保证"替换成"启动期报错 / 断言保证"
不要彻底放弃这层保证
```

**3. ini 是存储格式，不是编辑界面**

```text
日常增删标签走 Project Settings -> GameplayTags 面板
ini 文件只是面板落盘的格式，不建议手改
善用面板的 DevComment 字段
为语义不直观的标签写清楚用途
例如 Event.Combo.ChainWindow、Data.StaminaCost
```

**4. 标签层级匹配语义**

```text
RequiredTags / BlockedTags / RequiredOwnerTags / RequiredTargetTags
全部依赖 FGameplayTagContainer 的层级匹配行为

在正式大量填写标签前，先用一两个例子验证清楚：
State.Stunned 的容器是否匹配 State.Stunned.Heavy

确认后把结论写成 taxonomy 的一条约定
避免出现"明明加了 BlockedTags 为什么技能还能放"这类问题
```
