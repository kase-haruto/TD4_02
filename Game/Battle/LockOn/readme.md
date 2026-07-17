# ロックオンシステム設計案

この文書は実装前確認用です。既存コードを調査した結果に基づき、`PlayerLockOnController` を中心にしつつ、対象検索、対象選択、対象登録、移動、カメラ、UI を分離する方針をまとめます。

## 1. 調査した既存構造

### Player と更新順序

- `Player` は `PlayerBase` を継承している。
- `PlayerBase` は `PlayerInput input_`、`PlayerMotor motor_`、`PlayerDodge dodge_`、`PlayerAttack attack_` を所有している。
- `Player::Update(float dt)` の現在の主な順序は以下。
  1. HP/リスポーン確認
  2. ノックバック中なら早期 return
  3. クローン追従
  4. `input_.Update()`
  5. `ability_.Update(...)`
  6. `dodge_.Update(...)`
  7. `attack_.Update(...)`
  8. `motor_.Update(...)`
  9. `Actor::Update(dt)`
- ロックオン更新は、入力更新後、移動更新前に入れるのが自然。

### 入力管理

- 入力は `Game/Demo/Input/PlayerInput` が `PlayerInputState` を生成する。
- 実キーは `InputAction` と `BaseInputAction<InputAction>` のバインディングで扱う。
- 現在の `InputAction` は移動、攻撃、アビリティ、ダッシュのみ。
- ロックオンは `PlayerLockOnController` 内でキーコードを読まず、`PlayerInput` 側で `lockOnPressed`、`unlockPressed`、`switchLeftPressed`、`switchRightPressed` などの要求状態へ変換する。

### TransformRef

- SDK の `Engine/Scene/Reference/TransformReference.h` に `CalyxEngine::TransformRef` がある。
- `TransformRef` は `WorldTransform*` を保持せず、所有 `SceneObject` の GUID を `SceneObjectRef<SceneObject>` で保持する。
- 解決は `const WorldTransform* Resolve() const` または `Resolve(const ISceneObjectResolver&) const`。
- 無効判定は専用 `IsValid()` ではなく、実質的に `Resolve() != nullptr` で行う。
- 未設定判定は `IsAssigned()`。
- `Clear()` で参照を消せる。
- `to_json/from_json` は GUID のみを保存する。

### SceneObjectRef と破棄時の挙動

- `SceneObjectRef<T>` は GUID と `weak_ptr` キャッシュを持つ。
- `Resolve()` は現在の `SceneContext` の resolver から毎回 GUID を解決する。
- 対象が未設定、別シーン、削除済み、型不一致の場合は `nullptr` を返す。
- `shared_ptr` による対象延命はしない設計になっている。

### SceneObject / Actor / Registry

- `SceneObject` は `WorldTransform worldTransform_` を持ち、`GetWorldTransform()` で参照できる。
- `Actor` は `SceneObject` 系のゲーム内基底で、`CharacterMovementComponent` を持つ。
- オブジェクト生成・削除は `SceneContext::Instantiate/AddObject/RemoveObject` と `SceneObjectLibrary` が担当。
- `SceneObjectLibrary` は GUID で検索でき、`ResolveSceneObject()` を実装している。
- `EventBus` に `ObjectAdded` / `ObjectRemoved` が存在し、`SceneObjectLibrary` と `SceneContext` が利用している。
- ゲーム側のオブジェクト登録は `CALYX_OBJECT` と生成済み `GameObjectRegistry.generated.cpp` 経由。

### 敵オブジェクト

- 敵の基底は `BaseEnemy : Actor`。
- `BaseEnemy` は `IsDead()`、`TakeDamage()`、`EnemyStats`、移動/攻撃 Strategy を持つ。
- 死亡時は `SceneContext::Current()->RemoveObject(shared_from_this())` を呼ぶ。
- 現状、ロックオン対象としての共通インターフェースやコンポーネントは未実装。
- `Game/Battle/LockOn/ILockOnTarget.h` と `LockOnTargetComponent.h/.cpp` は存在するが、内容は未完成または空。

### カメラ

- `CameraPivot` は `SceneObject` で、`SceneObjectRef<Player>` を使って追従対象を解決する。
- メインカメラは `CameraManager::GetMain3d()` で取得している箇所がある。
- `WorldToScreen(const Vector3&, Vector2&)` が `MathUtil.h` に存在する。
- 初回実装ではカメラ挙動変更は最小化し、`ILockOnStateReader` を参照できる接続口だけ用意するのが安全。

### UI / HUD

- SDK に `Engine/Objects/2D/Hud/BaseHud`、`SpriteObject2d`、`SpriteSceneObject2d` がある。
- ロックオンマーカーは `EventBus` のロックオンイベントを購読し、`TransformRef` を保持して毎フレーム `WorldToScreen` で追従表示する形が合う。
- 今回の最初の実装ではイベント発行までを必須、HUD 実体は別オブジェクトとして追加可能にする。

### コリジョン / 空間検索

- 衝突は `Collider`、`BoxCollider`、`CollisionLayerUtil` が使われている。
- 調査範囲では汎用 Sphere Query / Overlap Query は見つからない。
- 初期実装は `LockOnTargetRegistry` 内の登録済み対象だけを走査し、`ILockOnTargetQuery` 差し替えで将来 Sphere Query に移行できる形にする。

### イベント機構

- SDK に `EventBus` がある。
- `EventBus::Subscribe<T>()` は RAII の `EventBus::Connection` を返す。
- `EventBus::Publish<T>()` で型安全に通知できる。
- ロックオン開始/解除/変更もこれに載せる。

### シリアライズ可能パラメータ

- ゲーム側は `CalyxEngine::SerializableObject` を継承した構造体で `AddField()`、`LoadParams()`、`ShowGui()` を使っている。
- `GetParamPath()` を実装すると `Resources/Params/...` 配下へ保存される。
- ロックオンの調整値は `LockOnSettings : SerializableObject` に集約する。
- `currentTarget_`、候補配列、タイマー、クールダウンなどのランタイム状態は `SerializableObject` に含めない。

## 2. 採用する設計

既存の 3D オブジェクト側には汎用 Component 所有機構が見当たらないため、初回実装では「コンポーネント風の小クラスを `BaseEnemy` など対象オブジェクトが所有する」方式を採用する。

```text
Player
└─ PlayerLockOnController
   ├─ ILockOnTargetQuery
   ├─ ILockOnTargetSelector
   └─ LockOnSettings

LockOnTargetComponent
└─ TransformRef targetTransform_

LockOnTargetRegistry
└─ TransformRef を持つ非所有エントリ

PlayerMotor
└─ ILockOnStateReader を参照

CameraPivot / LockOnIndicator
└─ ILockOnStateReader または EventBus イベントを参照
```

`PlayerLockOnController` は検索アルゴリズム、敵固有処理、移動、カメラ、UI を持たない。入力からの「要求」を受け、現在対象を `TransformRef` で保持し、状態変化をイベント発行する。

## 3. 追加予定ファイル

- `Game/Battle/LockOn/LockOnSettings.h`
- `Game/Battle/LockOn/ILockOnStateReader.h`
- `Game/Battle/LockOn/ILockOnTargetQuery.h`
- `Game/Battle/LockOn/ILockOnTargetSelector.h`
- `Game/Battle/LockOn/LockOnTargetEntry.h`
- `Game/Battle/LockOn/LockOnTargetRegistry.h/.cpp`
- `Game/Battle/LockOn/LockOnTargetComponent.h/.cpp`
- `Game/Battle/LockOn/DefaultLockOnTargetQuery.h/.cpp`
- `Game/Battle/LockOn/DefaultLockOnTargetSelector.h/.cpp`
- `Game/Battle/LockOn/PlayerLockOnController.h/.cpp`
- `Game/Battle/LockOn/LockOnEvents.h`

既存の空ファイル `ILockOnTarget.h/.cpp` と `LockOnTargetComponent.h/.cpp` は、ユーザー作業を壊さないよう内容を確認してから統合する。

## 4. 各クラスの責務

### PlayerLockOnController

- ロックオン開始/解除要求を受ける。
- 現在対象を `CalyxEngine::TransformRef currentTarget_` で保持する。
- 毎フレーム `Resolve() != nullptr` を確認する。
- 距離超過、対象無効、クールダウン、猶予時間を管理する。
- 左右切り替え要求を処理する。
- `ILockOnStateReader` を実装し、読み取り専用 API を公開する。
- `EventBus` へ開始/解除/変更イベントを発行する。

### LockOnTargetComponent

- 対象オブジェクトが所有する軽量データ。
- ロックオン位置の `TransformRef`、`isLockable`、`priority` を持つ。
- `Initialize(owner)` または明示 API で対象 Transform を設定する。
- `Register/Unregister` は所有者の `Initialize` / `Destroy` / 死亡処理で呼ぶ。

### LockOnTargetRegistry

- ロックオン可能対象の非所有リスト。
- エントリは `TransformRef` と priority などの値だけを保持する。
- 検索時に `Resolve() != nullptr` を確認し、無効なエントリを除外する。
- 登録解除漏れがあってもクラッシュしない。
- `ObjectRemoved` イベントまたは定期 prune で無効登録を削除する。

### DefaultLockOnTargetQuery

- `LockOnTargetRegistry` を使って候補を列挙する。
- 距離、カメラ前方、最大角度の粗いフィルタを行う。
- 将来、物理 Sphere Query や空間分割へ差し替えられる。

### DefaultLockOnTargetSelector

- 候補から最適対象を選ぶ。
- スコアは画面中心距離、カメラ正面角度、プレイヤー距離、priority で計算する。
- 左右切り替えでは現在対象のスクリーン X を基準に、左右方向の候補だけを評価する。

### LockOnIndicator

- 初回実装では任意。
- `EventBus` を購読して `TransformRef` を受け取り、`WorldToScreen` でマーカーを追従する。
- `PlayerLockOnController` から UI を直接触らない。

## 5. TransformRef の保持・解決・無効化

保持:

```cpp
CalyxEngine::TransformRef currentTarget_;
```

解決:

```cpp
const WorldTransform* target = currentTarget_.Resolve();
if (!target) {
    ClearTarget();
    return;
}
```

未設定:

```cpp
if (!currentTarget_.IsAssigned()) {
    return false;
}
```

解除:

```cpp
currentTarget_.Clear();
```

`TransformRef` は `shared_ptr` を保持しないため、敵や対象オブジェクトを延命しない。シーン切替や削除後は `Resolve()` が `nullptr` になり、ロックオン解除へ進む。

## 6. Player / 移動 / カメラ / UI との依存関係

- `Player` は `PlayerLockOnController` を所有する。
- `Player` は `TransformRef`、`ILockOnTargetQuery`、`ILockOnTargetSelector`、`LockOnSettings` だけを渡して初期化する。
- `PlayerLockOnController` へ `Player*` は渡さない。
- `PlayerMotor` は `ILockOnStateReader*` を持ち、ロックオン中だけ対象方向へ Y 軸回転する。
- `CameraPivot` は初回では挙動変更せず、必要なら `ILockOnStateReader` 接続口だけ追加する。
- UI は `EventBus` 経由で状態変化を受ける。

## 7. 検索方法

初期実装:

1. `LockOnTargetRegistry` の登録対象だけを取得。
2. `TransformRef::Resolve()` が失敗する対象は除外。
3. `playerPosition` から `searchRadius` 外の対象を除外。
4. `cameraForward` との角度が `maximumAngleDegrees` を超える対象を除外。
5. `WorldToScreen` が失敗する対象は低優先または除外。

将来拡張:

- `ILockOnTargetQuery` を物理 Sphere Query 実装へ差し替える。
- 遮蔽物判定用 Raycast を追加する。

## 8. 対象選択スコア

標準 selector は小さいほど良いスコアにする。

```text
score =
  screenCenterDistanceNormalized * screenCenterWeight
+ angleNormalized                * angleWeight
+ distanceNormalized             * distanceWeight
- priority                       * priorityWeight
```

画面中心への近さを最優先にし、距離だけで真横や画面端の敵が選ばれないようにする。カメラ後方は候補段階で除外する。

## 9. 左右切り替え

- 現在対象と候補をそれぞれ `WorldToScreen` する。
- 右切り替えは `candidate.x > current.x + epsilon` のみ。
- 左切り替えは `candidate.x < current.x - epsilon` のみ。
- スコアは以下を考慮する。
  - 現在対象からのスクリーン X 差
  - スクリーン Y 差
  - 画面中心からの距離
  - カメラ正面角度
  - プレイヤー距離
- `switchCooldownSeconds` 中は次の切り替えを無視する。
- 現在対象と同じ GUID の候補は除外する。

## 10. ロックオン解除条件

初期実装で対応:

- ユーザー解除要求。
- `currentTarget_.Resolve() == nullptr`。
- `currentTarget_` が未設定。
- 対象が `breakDistance` より離れた。
- Registry 側で対象が lockable ではないと判定された。
- シーン切替で `Resolve()` できなくなった。

後続対応:

- 一定時間画面外。
- 一定時間遮蔽物あり。
- 画面外/遮蔽物の猶予は `lostTargetGraceSeconds` を使用。

## 11. シリアライズ対象と非対象

シリアライズ対象:

- `LockOnSettings`
  - `searchRadius`
  - `breakDistance`
  - `maximumAngleDegrees`
  - score weight 群
  - `switchCooldownSeconds`
  - `lostTargetGraceSeconds`
  - `playerTurnSpeed` など移動連携値
- `LockOnTargetComponent`
  - `targetTransform`
  - `isLockable`
  - `priority`

シリアライズ非対象:

- `PlayerLockOnController::currentTarget_`
- 候補 vector
- クールダウンタイマー
- lost target タイマー
- 現在ロックオン中フラグ
- EventBus connection

## 12. 実装時の注意点

- `TransformRef` に `IsValid()` は見当たらないため、`Resolve() != nullptr` を無効判定として使う。
- `TransformRef` が指せるのは現在の設計では SceneObject の `WorldTransform`。ボス部位や頭など任意子 Transform は、初期実装では「ロックオン用 SceneObject を子として置く」方式が安全。
- 3D 側に汎用 Component コンテナが見当たらないため、`LockOnTargetComponent` は `BaseEnemy` 等がメンバとして所有する。将来 SDK に ComponentRef が入った場合に移行する。
- 既存 `EventBus::Publish` は購読中の vector を直接走査するため、イベントハンドラ内で同じイベントの購読解除/追加を多用しない。
- `PlayerBase::Update` と `Player::Update` の両方に入力・移動更新があるが、実際の `Player` は `Player::Update` を使う。統合は `Player::Update` 側を主対象にする。
- `TD4_02.vcxproj` には現在 `ILockOnTarget.cpp` / `.h` がルート相対の誤った項目として入っているように見える。実装時にプロジェクト項目を整理する必要がある。

## 13. 実装ステップ案

1. `LockOnSettings`、Entry、Event、Reader/Query/Selector interfaces を追加。
2. `LockOnTargetRegistry` と `DefaultLockOnTargetQuery` を追加。
3. `DefaultLockOnTargetSelector` を追加。
4. `PlayerLockOnController` を追加。
5. `PlayerInputState` と `InputAction` にロックオン要求を追加。
6. `Player` が controller を所有し、入力更新後・移動前に要求と更新を流す。
7. `PlayerMotor` に `ILockOnStateReader` 接続口を追加し、ロックオン中の Y 軸回転へ対応。
8. `BaseEnemy` に `LockOnTargetComponent` を所有させ、初期化時登録、死亡/破棄時解除を行う。
9. `EventBus` イベントを発行し、将来 HUD が購読できる形にする。
10. vcxproj / filters / reflection 登録を更新してビルド確認する。

## 14. 動作確認予定

- 対象なしでロックオン要求してもクラッシュしない。
- 範囲内かつ画面中央に近い対象が選ばれる。
- カメラ後方の対象が選ばれない。
- 左右切り替えで同一対象に戻らない。
- 切り替えクールダウンが効く。
- 対象死亡・削除後、次フレームで解除される。
- Registry に無効エントリが残っても検索で除外される。
- `shared_ptr` で対象を延命していない。
- ロックオン状態がシーンファイルへ保存されない。
- 通常時の移動・攻撃・回避が壊れていない。

## 15. 未対応または確認が必要な点

- 物理 Sphere Query / Raycast は SDK 調査範囲でロックオン用途に使える API が見つかっていない。
- ロックオンマーカー用 HUD はイベント設計までを先に入れ、見た目とアセットは別途確定する。
- 任意の骨やモデル内部ノードを直接 `TransformRef` で指す仕組みは現状見当たらない。必要ならロックオン用の子 SceneObject を置く。
- 既存の `Game/Battle/LockOn` 未追跡ファイルは空に近いため、実装時に内容を統合してよいか確認が必要。
