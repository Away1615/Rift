# Rift TwinSword / GAS 改造记录

## 目标

本记录用于说明当前 `feat/new_develop` 分支如何从早期 TwinSword 原型演进到现在的 UE5 / GAS 战斗框架，以及后续继续落地 TwinSword 时应遵守的设计边界。

当前核心方向：

- 双剑是近战压制职业，核心体验是快速连招、体力管理、完美闪避、剑意强化和大招终结。
- Space 是闪避 / 完美闪避 / 连段取消。
- E 是剑气开关，持续强化普通连段。
- Shift 是剑影同调，消耗剑意值进入短时间压制强化。
- Q 是大招，不参与普通连段分支，只通过攻击、连段终结、完美闪避、破防和击杀等行为充能。
- 剑意值和 Q 大招槽必须分离：剑意值服务短循环，Q 大招槽服务长循环爆发奖励。

## 当前设计定案

### 输入定位

| 输入 | 定位 | 消耗 |
| --- | --- | --- |
| 左键 | 快速轻攻击，积累剑意值和 Q 充能 | 无 |
| 右键 | 更重的压制攻击、突进、削韧 | Stamina |
| 重击 | 爆发收尾 | Stamina + 剑意值 |
| Space | 闪避 / 完美闪避 / 连段取消 | Stamina |
| E | 剑气开关，强化普通连段 | 每次攻击消耗剑意值 |
| Shift | 剑影同调，短时间强化攻击范围和伤害 | 剑意值 |
| Q | 双生剑域，大招终结 | Q 大招槽 |

### 第一版连段树

```text
左键轻连段：
L1 -> L2 -> L3 -> L4

左键转右键压制：
L1 -> L2 -> R1 -> R2

空闲右键起手：
R0 -> R1 -> R2

重击派生：
L1 -> L2 -> 左键重击
L1 -> L2 -> 右键重击
L1 -> L2 -> R1 -> 右键重击

闪避取消：
任意攻击段 -> Space -> 连段重置
```

第一版 Montage Section：

```text
Primary_1
Primary_2
Primary_3
Primary_4

Secondary_Opener_R0
Secondary_1_R1
Secondary_2_R2

Heavy_Primary
Heavy_Secondary
```

空闲右键进入 `R0`。`L2` 后右键直接进入 `R1`。`R1 / R2` 后续共用，避免为每个状态做独立连招系统。

## 分支演进记录

当前分支：`feat/new_develop`

关键提交脉络：

- `1bae33a4 feat: TwinSword prototype 60 %`：早期 TwinSword 原型。
- `3ab5cebc feat: Support basic twinsword skill control`：开始形成基础双剑技能控制。
- `f0090328 feat: Add GAS ability grant pipeline`：引入玩家 ASC、AttributeSet 和职业 DataAsset 授权 Ability 的基本管线。
- `bac8ac81 feat: Add primary attack GAS montage pipeline`：左键攻击接入 GAS Ability 与 Montage。
- `fb6edf10 feat: four-hit primary combo with input buffering`：左键四段连击、输入缓冲、Combo Window、Chain Point Notify。
- `728f7089 feat: soft target assist component with assisted facing integration`：加入软锁目标选择和攻击段辅助转向。
- `762e5f6c feat: server-authoritative melee combat with GAS damage and hitstop`：近战 Trace 改为服务端权威，伤害通过 GE SetByCaller，下发 HitStop 和 GameplayCue。
- `3f95325b feat: enemy poise system with directional hit reactions`：敌人 Health / Poise AttributeSet、韧性伤害、受击方向 Montage Section。
- `524d94b4 feat: directional melee hit camera shake`：按连击段配置命中镜头震动方向。

本轮未提交改造：

- `GDD.md` 重写 TwinSword 设计，明确剑意值与 Q 大招槽分离，补入 `L1/L2/R0/R1/R2` 第一版连段结构。
- 增加 `URiftPlayerAttributeSet`，玩家 Health / Stamina 独立于敌人 AttributeSet。
- `ABasePlayerState` 改为持有 `URiftPlayerAttributeSet`。
- `UPlayerCommonConfig` 增加 `StaminaRegenRate`，玩家职业配置应用 Health / Stamina 初值，并应用无限期 Stamina Regen GE。
- 增加 `UGE_StaminaRegen` 与 `UGE_DodgeCost`。
- `UPlayerCombatConfig` 增加 Dodge Montage、Stamina 消耗、i-frame 延迟、i-frame 持续时间、Perfect Dodge Window。
- 增加 `UGA_Dodge`：Space 触发，消耗 Stamina，取消攻击连段，按 WASD / 面向决定方向，播放 Dodge Montage，使用 `State.Dodging` 表示无敌窗口。
- `GE_MeleeDamage` 使用 `CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>`，通过 `State.Dodging` 阻止伤害应用。
- `UPlayerInputConfig` 增加 `CoreAction`，`ABasePlayerController` 将 CoreAction 映射到 `InputTag.Core`，`UGA_Dodge` 使用 `InputTag.Core` 激活。

## 当前架构状态

### 输入到 Ability

`ABasePlayerController` 负责读取 `UPlayerInputConfig`：

- `PrimaryAttackAction` -> `InputTag.Attack.Primary`
- `CoreAction` -> `InputTag.Core`

`URiftAbilitySystemComponent::AbilityInputTagPressed` 负责：

- 如果已有匹配输入 Tag 的激活 Ability，则调用 `AbilitySpecInputPressed`。
- 如果没有激活 Ability，则通过输入 Tag 尝试激活 Ability。

### 玩家战斗

- `UGA_ComboAttack` 负责左键主连段。
- `UGA_Dodge` 负责 Space 闪避。
- `APlayerCharacter` 持有软锁组件、武器 Trace 组件、职业配置、武器 Mesh、Stamina Regen GE Handle、Perfect Dodge Window 状态。
- `URiftWeaponTraceComponent` 负责服务端权威武器 Sweep，避免同一 Hit Window 内重复命中同一敌人。

### 敌人与伤害

- 敌人 ASC 使用 `URiftEnemyAttributeSet`。
- 近战伤害通过 `UGE_MeleeDamage` 写入敌人的 `Damage` 和 `PoiseDamage` meta attribute。
- `URiftEnemyAttributeSet::PostGameplayEffectExecute` 扣 Health、扣 Poise，并通知 `AEnemyCharacter` 播放受击或硬直。
- `AEnemyCharacter` 根据攻击来源方向选择 Front / Back / Left / Right Montage Section。

## 已知未落地项

- `DA_PlayerInput` 需要在编辑器里把 `CoreAction` 指到 `IA_Core`。
- TwinSword 职业 DataAsset 需要授权 `GA_Dodge`。
- TwinSword CombatConfig 需要配置 `DodgeMontage`。
- 当前代码只有玩家打敌人的伤害路径；敌人打玩家路径还未落地，所以 Perfect Dodge 只能开启窗口，尚未被敌人攻击检测消费。
- `IsPerfectDodgeWindowActive()` 和 `GetPerfectDodgeOrigin()` 是为后续敌人攻击判定保留的接口，目前没有调用方。
- E、Shift、Q、剑意值、Q 大招槽、右键 R0/R1/R2、重击派生还停留在 GDD 设计层，没有完整 C++ 实现。

## 后续落地顺序

1. 先清理当前空实现和未使用脚手架，避免继续堆叠在不明确的代码上。
2. 完成敌人攻击玩家的 GAS 伤害路径，让 `State.Dodging` 和 Perfect Dodge Window 有真实消费点。
3. 扩展输入配置：SecondaryAction、PrimaryHeavy、SecondaryHeavy、SkillE、SkillShift、UltimateQ。
4. 将 `UGA_ComboAttack` 从单一 Primary Montage 扩展为第一版连段树：`L1/L2/L3/L4`、`R0/R1/R2`、`Heavy_Primary`、`Heavy_Secondary`。
5. 加入剑意值 Attribute 或职业资源组件，并让攻击命中、完美闪避、E、Shift、重击围绕剑意值形成短循环。
6. 加入 Q 大招槽，独立于剑意值；Q 只由主动进攻、完美闪避、破防和击杀充能。
7. 实现 E 剑气开关。
8. 实现 Shift 剑影同调。
9. 实现 Q 双生剑域。

## 代码约束

- UE 构造函数里创建默认子对象必须使用 `CreateDefaultSubobject`，不要用 `NewObject` 或 `FindOrAddComponent` 创建默认子对象。
- `NewObject` 只用于运行时动态对象，例如当前运行时挂载武器 Mesh。
- 不要让 Q 消耗剑意值。
- 不要把 Q 当普通追击 / 反击技使用。
- Space 取消普通攻击后必须重置连段。
- 右键第一版只额外增加 `R0` 起手，后续 `R1/R2` 共用，避免过度设计。
- UE 编译验证直接用 UBT，不使用 `Build.bat`。
- 当前团队偏好避免匿名函数；新增定时器回调优先使用成员函数。

## 最近验证记录

- `GE_MeleeDamage` 已从错误的默认子对象创建方式改为 `CreateDefaultSubobject`。
- 全局检查过 `NewObject` / `CreateDefaultSubobject`：默认子对象均使用 `CreateDefaultSubobject`；剩余 `NewObject<UStaticMeshComponent>` 是运行时动态武器 Mesh 创建。
- 添加 `CoreAction` 后 UHT 已通过；完整 UBT 被当前 Editor 的 Live Coding 锁阻止。关闭 Editor 或按 `Ctrl+Alt+F11` 后需要重新跑 UBT。
- `git status` 在当前环境会被 LFS clean filter 的 `.git/lfs/tmp/... Access is denied` 卡住；读取 git log 和限定路径 diff 可用。
