#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "MC500_InputComponent.h"

MC500_InputComponent::MC500_InputComponent()
{
    // 液晶画面を可視化
    addAndMakeVisible (lcdArea);

    // 1. スライダー設定
    alphaDialSlider.setLookAndFeel (&dialLookAndFeel);
    alphaDialSlider.setRange (0.0, 960.0);
    alphaDialSlider.setValue (480.0);
    addAndMakeVisible (alphaDialSlider);

    // 2. 左ボタン設定
    leftButton.setLookAndFeel (&leftArrowLookAndFeel);
    rightButton.setLookAndFeel (&rightArrowLookAndFeel);
    setupButton (leftButton, juce::String::fromUTF8 ("←"));
    setupButton (rightButton, juce::String::fromUTF8 ("→"));

    // 3. 中央ボタン設定（2列4行分 = 8個）
    std::vector<juce::String> centerLabels {
        "MIDI", "EDIT",
        "FUNC", "MICRO",
        "MODE", "AVAIL",
        "SHIFT", "SPACE"
    };
    for (const auto& label : centerLabels)
    {
        auto* btn = new juce::TextButton();
        setupButton (*btn, label);
        centerButtons.add (btn);
    }

    // 4. 右テンキーボタン設定 (10個)
    std::vector<juce::String> numLabels {
        "7", "8", "9",
        "4", "5", "6",
        "1", "2", "3",
        "0"
    };
    for (const auto& label : numLabels)
    {
        auto* btn = new juce::TextButton();
        setupButton (*btn, label);
        numButtons.add (btn);
    }
    setupButton (enterButton, "ENTER");

    alphaDialSlider.slider.setRange (-1000.0, 1000.0, 1.0);
    alphaDialSlider.slider.setValue (0.0, juce::dontSendNotification);

    // ==============================================================================
    // ★ アルファダイヤルの操作（マウスホイール・スピード調整完全版）
    // ==============================================================================
    alphaDialSlider.slider.onValueChange = [this]()
    {
        double rawDelta = alphaDialSlider.slider.getValue();
        if (juce::approximatelyEqual (rawDelta, 0.0)) return;

        // 🌟【重要】マウスホイールの大きな移動量を、1ステップ（1.0 または -1.0）に丸める
        // これにより、ホイールを大きく回しても、キーボードと同じ滑らかな挙動に統一されます
        int intDelta = (rawDelta > 0.0) ? 1 : -1;

        // 🌟【スピード調整】1ステップあたりの回転角度を「3度」に設定
        // 3度 ＝ 3.0f * (3.14159265f / 180.0f) ≒ 0.05236f
        float angleStep = 0.05236f;

        // シフトキーが押されていたら、見た目の回転も10倍（30度）にする
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown())
        {
            intDelta *= 10;
        }

        // 見た目の角度を累積増減
        dialVisualAngle += angleStep * static_cast<float>(intDelta);

        // LookAndFeelに伝えて再描画
        dialLookAndFeel.setVisualAngle (dialVisualAngle);
        alphaDialSlider.slider.repaint();

        // 液晶の数値を更新
        switch (currentDialMode)
        {
            case DialTargetMode::None:
                break;
            case DialTargetMode::Measure:
            {
                int numNew = lcdArea.getMeasure() + intDelta;
                lcdArea.setMeasure (juce::jlimit (0, 999, numNew));
                break;
            }
            case DialTargetMode::Beat:
            {
                int numNew = lcdArea.getBeat() + intDelta;
                lcdArea.setBeat (juce::jlimit (0, 9, numNew));
                break;
            }
            case DialTargetMode::Clock:
            {
                int numNew = lcdArea.getClock() + intDelta;
                lcdArea.setClock (juce::jlimit (0, 999, numNew));
                break;
            }
            case DialTargetMode::StepTime:
            {
                int numNew = lcdArea.getStepTime() + intDelta;
                lcdArea.setStepTime (juce::jlimit (0, 999, numNew));
                break;
            }
            case DialTargetMode::Note:
            {
                int numNew = lcdArea.getNoteNumber() + intDelta;
                lcdArea.setNoteNumber (juce::jlimit (0, 127, numNew));
                break;
            }
            case DialTargetMode::Velocity:
            {
                int numNew = lcdArea.getVelocity() + intDelta;
                lcdArea.setVelocity (juce::jlimit (0, 127, numNew));
                break;
            }
            case DialTargetMode::GateTime:
            {
                int numNew = lcdArea.getGateTime() + intDelta;
                lcdArea.setGateTime (juce::jlimit (0, 9999, numNew));
                break;
            }
            default:
                break;
        }

        // スライダーを中央に戻す
        alphaDialSlider.slider.setValue (0.0, juce::dontSendNotification);
    };

    // ▽▽▽ 追記：左矢印ボタンの処理 ▽▽▽
    leftButton.onClick = [this]()
    {
        // 現在のモードを取得
        auto mode = lcdArea.getEditMode(); // ※LcdComponentにゲッターが無い場合は後述の修正参照

        // モードを左（逆順）に切り替える
        int modeInt = static_cast<int>(mode);
        modeInt--;
        if (modeInt < static_cast<int>(LcdComponent::EditMode::Measure))
            modeInt = static_cast<int>(LcdComponent::EditMode::GateTime); // 最後の項目へループ

        lcdArea.setEditMode(static_cast<LcdComponent::EditMode>(modeInt));
        currentDialMode = static_cast<DialTargetMode>(modeInt);
    };

    // ▽▽▽ 追記：右矢印ボタンの処理 ▽▽▽
    rightButton.onClick = [this]()
    {
        auto mode = lcdArea.getEditMode();

        // モードを右（正順）に切り替える
        int modeInt = static_cast<int>(mode);
        modeInt++;
        if (modeInt > static_cast<int>(LcdComponent::EditMode::GateTime))
            modeInt = static_cast<int>(LcdComponent::EditMode::Measure); // 最初の項目へループ

        lcdArea.setEditMode(static_cast<LcdComponent::EditMode>(modeInt));
        currentDialMode = static_cast<DialTargetMode>(modeInt);
    };

    // ==============================================================================
    // ★ 右のテンキー配列 (0 〜 9 キー) のクリック連動
    // ==============================================================================
    // テンキーで数値をダイレクトに入力するためのバッファ変数などを保持すると実機に近づきます。
    // まずは「どのボタンが物理的に押されたか」を液晶のノート番号やベロシティに仮で反映させて確認します。
    for (auto* btn : numButtons)
    {
        if (btn != nullptr)
        {
            btn->onClick = [this, btn]()
            {
                juce::String numStr = btn->getButtonText();
                int numValue = numStr.getIntValue();

                switch (currentDialMode)
                {
                    case DialTargetMode::None:
                        break;
                    case DialTargetMode::Measure:
                    {
                        int numNew = (lcdArea.getMeasure() * 10 + numValue) % 1000;
                        lcdArea.setMeasure (juce::jlimit (0, 999, numNew));
                        break;
                    }
                    case DialTargetMode::Beat:
                    {
                        int numNew = (lcdArea.getBeat() * 10 + numValue) % 10;
                        lcdArea.setBeat (juce::jlimit (0, 9, numNew));
                        break;
                    }
                    case DialTargetMode::Clock:
                    {
                        int numNew = (lcdArea.getClock() * 10 + numValue) % 1000;
                        lcdArea.setClock (juce::jlimit (0, 999, numNew));
                        break;
                    }
                    case DialTargetMode::StepTime:
                    {
                        int numNew = (lcdArea.getStepTime() * 10 + numValue) % 1000;
                        lcdArea.setStepTime (juce::jlimit (0, 999, numNew));
                        break;
                    }
                    case DialTargetMode::Note:
                    {
                        int numNew = (lcdArea.getNoteNumber() * 10 + numValue) % 1000;
                        lcdArea.setNoteNumber (juce::jlimit (0, 999, numNew));
                        break;
                    }
                    case DialTargetMode::Velocity:
                    {
                        int numNew = (lcdArea.getVelocity() * 10 + numValue) % 1000;
                        lcdArea.setVelocity (juce::jlimit (0, 999, numNew));
                        break;
                    }
                    case DialTargetMode::GateTime:
                    {
                        int numNew = (lcdArea.getGateTime() * 10 + numValue) % 10000;
                        lcdArea.setGateTime (juce::jlimit (0, 9999, numNew));
                        break;
                    }
                    default:
                        break;
                }
            };
        }
    }

    // ==============================================================================
    // ★ 中央のボタン群に「ダイヤル対象切り替え」の役割を割り当てる
    // ==============================================================================
    // ※ centerButtonsの生成順（インデックス）に合わせて適宜割り当てを調整してください。

    // 【1番目のボタン】 押すとダイヤルが「Step Time」操作モードになる
    if (centerButtons.size() > 0 && centerButtons[0] != nullptr)
    {
        centerButtons[0]->onClick = [this]()
        {
            currentDialMode = DialTargetMode::StepTime;
            // ダイヤルの現在の物理的な値を、液晶の現在のStepTimeの値（または初期値など）に同期させる
            // ※ここでは仮に初期設定として標準的な4分音符(96)などの値をセットしています
            lcdArea.setEditMode (LcdComponent::EditMode::StepTime); // 🌟液晶に通知
            alphaDialSlider.slider.setValue (96, juce::dontSendNotification);
        };
    }

    // 【2番目のボタン】 押すとダイヤルが「Gate Time」操作モードになる
    if (centerButtons.size() > 1 && centerButtons[1] != nullptr)
    {
        centerButtons[1]->onClick = [this]()
        {
            currentDialMode = DialTargetMode::GateTime;
            lcdArea.setEditMode (LcdComponent::EditMode::GateTime); // 🌟液晶に通知
            alphaDialSlider.slider.setValue (96, juce::dontSendNotification);
        };
    }

    // 【3番目のボタン】 押すとダイヤルが「Velocity」操作モードになる
    if (centerButtons.size() > 2 && centerButtons[2] != nullptr)
    {
        centerButtons[2]->onClick = [this]()
        {
            currentDialMode = DialTargetMode::Velocity;
            lcdArea.setEditMode (LcdComponent::EditMode::Velocity); // 🌟液晶に通知
            alphaDialSlider.slider.setValue (64, juce::dontSendNotification); // Velocity初期値
        };
    }

    setWantsKeyboardFocus(true);
}

MC500_InputComponent::~MC500_InputComponent()
{
    // ★ ヌルポインタをセットして LookAndFeel の参照を安全に外す（JUCEの決まり文句）
    alphaDialSlider.setLookAndFeel (nullptr);
    leftButton.setLookAndFeel (nullptr);
    rightButton.setLookAndFeel (nullptr);
}

void MC500_InputComponent::paint (juce::Graphics& g)
{
    // 背景色
    g.fillAll (juce::Colours::darkgrey);

    g.setFont (12.0f);
    //g.setColour (juce::Colours::black);
    g.setColour (juce::Colours::white);
}

void MC500_InputComponent::resized()
{
    // 全体の描画エリア（外側から15マス減らす）
    auto totalArea = getLocalBounds().reduced (15);

    // ==============================================================================
    // ★ 全体を上下2段に分割（上部20%を液晶、残りをコントローラー群にする）
    // ==============================================================================
    auto lcdBounds = totalArea.removeFromTop (totalArea.getHeight() * 0.20f);
    lcdArea.setBounds (lcdBounds);

    totalArea.removeFromTop (15); // 液晶とボタン群の間のスキマ
    auto area = totalArea;       // コントローラーエリア

    // -------------------------------------------------------------
    // 以下、従来の3ブロック分割計算（コントローラーエリア内で実行）
    // -------------------------------------------------------------
    // 計算しやすいように、全体の幅を「左:中央:右 = 5:5:7」の割合で綺麗にカットします
    int totalUnit = 5 + 5 + 7; // = 17
    int singleWidth = area.getWidth() / totalUnit;

    auto leftArea   = area.removeFromLeft (singleWidth * 5);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto centerArea = area.removeFromLeft (singleWidth * 5);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto rightArea  = area; // 残りが右セクション

    // -------------------------------------------------------------
    // 1. 【左セクション】の配置 (直接配置)
    // -------------------------------------------------------------
    // 上70%をダイヤル、下30%をボタン行にする
    auto dialArea = leftArea.removeFromTop (leftArea.getWidth());
    alphaDialSlider.setBounds (dialArea.reduced(10)); // 少し小さくして丸を綺麗に見せる

    leftArea.removeFromTop (8); // 縦の隙間

    // ← と → ボタンを横並びに分割
    auto leftButtonArea = leftArea.removeFromLeft (leftArea.getWidth() / 2).reduced(2, 0);
    auto rightButtonArea = leftArea.reduced(2, 0);
    leftButton.setBounds (leftButtonArea);
    rightButton.setBounds (rightButtonArea);

    // -------------------------------------------------------------
    // 2. 【中央セクション】の配置 (2列4行 Grid)
    // -------------------------------------------------------------
    juce::Grid centerGrid;
    centerGrid.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (1)), juce::Grid::TrackInfo (juce::Grid::Fr (1)) };
    for (int i = 0; i < 4; ++i)
        centerGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    centerGrid.columnGap = juce::Grid::Px (6);
    centerGrid.rowGap    = juce::Grid::Px (6);

    for (auto* btn : centerButtons)
        centerGrid.items.add (juce::GridItem (*btn));

    centerGrid.performLayout (centerArea);

    // -------------------------------------------------------------
    // 3. 【右セクション】の配置 (テンキー変則 Grid)
    // -------------------------------------------------------------
    juce::Grid rightGrid;
    rightGrid.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1))
    };
    for (int i = 0; i < 4; ++i)
        rightGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    rightGrid.columnGap = juce::Grid::Px (6);
    rightGrid.rowGap    = juce::Grid::Px (6);

    // 7,8,9, 4,5,6, 1,2,3, 0
    for (auto* btn : numButtons)
        rightGrid.items.add (juce::GridItem (*btn));

    // ENTER (ENTERは2マス結合)
    rightGrid.items.add (juce::GridItem (enterButton).withArea (4, 2, 5, 4));

    rightGrid.performLayout (rightArea);
}

void MC500_InputComponent::setupButton (juce::TextButton& btn, const juce::String& text)
{
    btn.setButtonText (text);
    btn.setConnectedEdges (0);
    addAndMakeVisible (btn); // ここで親コンポーネントへ確実に登録
}

// 画面をクリックした時に、確実にキーボード入力をこの画面に引き戻す対策
void MC500_InputComponent::mouseDown (const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    grabKeyboardFocus();
}

// キーボードが押された時の処理
bool MC500_InputComponent::keyPressed (const juce::KeyPress& key)
{
    // ==============================================================================
    // 1. アルファダイヤル（無限回転ノブ）の操作 (リセット同期型)
    // ==============================================================================

    // 【右回転】
    if (key.getKeyCode() == juce::KeyPress::upKey || key.getKeyCode() == ']' || key.getKeyCode() == '}')
    {
        double step = juce::ModifierKeys::getCurrentModifiers().isShiftDown() ? 10.0 : 1.0;

        // 現在の値（通常は0.0）にステップを加算してイベントを発火させる
        alphaDialSlider.slider.setValue (alphaDialSlider.slider.getValue() + step, juce::sendNotificationSync);
        return true;
    }

    // 【左回転】
    if (key.getKeyCode() == juce::KeyPress::downKey || key.getKeyCode() == '[' || key.getKeyCode() == '{')
    {
        double step = juce::ModifierKeys::getCurrentModifiers().isShiftDown() ? 10.0 : 1.0;

        // 現在の値（通常は0.0）からステップを減算してイベントを発火させる
        alphaDialSlider.slider.setValue (alphaDialSlider.slider.getValue() - step, juce::sendNotificationSync);
        return true;
    }

    // ◀ボタン
    if (key.getKeyCode() == juce::KeyPress::leftKey)
    {
        leftButton.triggerClick();
        return true;
    }

    // ▶ボタン
    if (key.getKeyCode() == juce::KeyPress::rightKey)
    {
        rightButton.triggerClick();
        return true;
    }

    // ==============================================================================
    // 2. 右のテンキー配列 (0 〜 9 キー) の操作 【レイアウト順に合わせた修正版】
    // ==============================================================================
    // キーボードの数字（0〜9）から、numButtons内のインデックスへ変換するマップ
    // 7->0, 8->1, 9->2, 4->3, 5->4, 6->5, 1->6, 2->7, 3->8, 0->9
    int targetIndex = -1;

    if      (key.isKeyCode ('7') || key.isKeyCode (juce::KeyPress::numberPad7)) targetIndex = 0;
    else if (key.isKeyCode ('8') || key.isKeyCode (juce::KeyPress::numberPad8)) targetIndex = 1;
    else if (key.isKeyCode ('9') || key.isKeyCode (juce::KeyPress::numberPad9)) targetIndex = 2;
    else if (key.isKeyCode ('4') || key.isKeyCode (juce::KeyPress::numberPad4)) targetIndex = 3;
    else if (key.isKeyCode ('5') || key.isKeyCode (juce::KeyPress::numberPad5)) targetIndex = 4;
    else if (key.isKeyCode ('6') || key.isKeyCode (juce::KeyPress::numberPad6)) targetIndex = 5;
    else if (key.isKeyCode ('1') || key.isKeyCode (juce::KeyPress::numberPad1)) targetIndex = 6;
    else if (key.isKeyCode ('2') || key.isKeyCode (juce::KeyPress::numberPad2)) targetIndex = 7;
    else if (key.isKeyCode ('3') || key.isKeyCode (juce::KeyPress::numberPad3)) targetIndex = 8;
    else if (key.isKeyCode ('0') || key.isKeyCode (juce::KeyPress::numberPad0)) targetIndex = 9;

    // 該当する数字キーが押されていた場合、正しいボタンを発火させる
    if (targetIndex != -1)
    {
        if (targetIndex < numButtons.size() && numButtons[targetIndex] != nullptr)
        {
            numButtons[targetIndex]->triggerClick();
            return true;
        }
    }

    // ENTERキー (キーボードの Enter / Return キー、またはテンキーの Enter)
    if (key.getKeyCode() == juce::KeyPress::returnKey)
    {
        enterButton.triggerClick();
        return true;
    }

    // ==============================================================================
    // 4. 中央のボタン配列 (MODE, EDIT, F1〜F4 など) の操作
    // ==============================================================================
    // 例：中央の1番目のボタンを 'M' キーに対応させる場合
    if (key.isKeyCode ('m') || key.isKeyCode ('M'))
    {
        if (centerButtons.size() > 0 && centerButtons[0] != nullptr) { centerButtons[0]->triggerClick(); return true; }
    }

    // 例：中央の2番目のボタンを 'E' キーに対応させる場合
    if (key.isKeyCode ('e') || key.isKeyCode ('E'))
    {
        if (centerButtons.size() > 1 && centerButtons[1] != nullptr) { centerButtons[1]->triggerClick(); return true; }
    }

    // PCのファンクションキー F1 〜 F4 と連動
    if (key == juce::KeyPress::F1Key) { if (centerButtons.size() > 2 && centerButtons[2] != nullptr) { centerButtons[2]->triggerClick(); return true; } }
    if (key == juce::KeyPress::F2Key) { if (centerButtons.size() > 3 && centerButtons[3] != nullptr) { centerButtons[3]->triggerClick(); return true; } }
    if (key == juce::KeyPress::F3Key) { if (centerButtons.size() > 4 && centerButtons[4] != nullptr) { centerButtons[4]->triggerClick(); return true; } }
    if (key == juce::KeyPress::F4Key) { if (centerButtons.size() > 5 && centerButtons[5] != nullptr) { centerButtons[5]->triggerClick(); return true; } }

    return false; // 割り当てていないその他のキーはスルー
}

void MC500_InputComponent::visibilityChanged()
{
    if (isVisible())
    {
        // 画面に表示されたら、100ミリ秒後に1回だけ実行するタイマーを起動
        // これにより、JUCEの画面接続処理がすべて終わるのを安全に待ちます
        startTimer (100);
    }
    else
    {
        // 非表示になったらタイマーを止める（安全のため）
        stopTimer();
    }
}

void MC500_InputComponent::timerCallback()
{
    // 1回だけ実行したいので、即座にタイマーを停止
    stopTimer();

    // 全ての構築が終わったこのタイミングであれば、絶対にアサーションを出さずにフォーカスを取れます
    grabKeyboardFocus();
}
