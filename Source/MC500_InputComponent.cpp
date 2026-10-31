#include "MC500_InputComponent.h"
#include "JuceStepsMidiManager.h"
#include "LcdComponent.h"
#include "Multi10KeyButton.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

MC500_InputComponent::MC500_InputComponent()
{
    // 初期値
    stepEvent.measure = 1;
    stepEvent.beat = 1;
    stepEvent.tick = 0;
    stepEvent.stepTime = 24;
    stepEvent.noteNumber = 60;
    stepEvent.velocity = 64;
    stepEvent.gateTime = stepEvent.stepTime * 0.75f;    // 長さの75%

    // 液晶画面を可視化
    addAndMakeVisible (lcdArea);

    // 1. スライダー設定
    alphaDialSlider.setLookAndFeel (&dialLookAndFeel);
    alphaDialSlider.setRange (0.0, 960.0);
    alphaDialSlider.setValue (480.0);
    addAndMakeVisible (alphaDialSlider);

    // 2. 左ボタン設定
    leftButton.setLookAndFeel (&leftArrowLookAndFeel);
    leftButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(leftButton);
    rightButton.setLookAndFeel (&rightArrowLookAndFeel);
    rightButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(rightButton);

    // 3. 中央ボタン設定（2列4行分 = 8個）
    std::vector<juce::String> centerLabels {
        "REC", "PAUSE",
        "FUNC", "MICRO",
        "RESET", "STOP",
        "SHIFT"
    };
    for (const auto& label : centerLabels)
    {
        auto* btn = new juce::TextButton();
        btn->setWantsKeyboardFocus(false);
        setupButton (*btn, label);
        if (label == "REC" || label == "SHIFT")
        {
            btn->setClickingTogglesState(true);
        }
        centerButtons.add (btn);
    }
    centerButtons[0]->setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);

    // 4. 右テンキーボタン設定 (10個)
    const std::array<int, 10> number
    {
        7, 8, 9, 4, 5, 6, 1, 2, 3, 0
    };
    for (auto number1 : number)
    {
        auto* btn = new Multi10KeyButton(number1);
        btn->setConnectedEdges (0);
        btn->setWantsKeyboardFocus(false);
        addAndMakeVisible (btn);
        numberButtons.add (btn);
    }
    enterButton.setWantsKeyboardFocus(false);
    setupButton (enterButton, "ENTER");

    alphaDialSlider.slider.setRange (-1000.0, 1000.0, 1.0);
    alphaDialSlider.slider.setValue (0.0, juce::dontSendNotification);

    // ==============================================================================
    // 左ボタンのエリア
    // ==============================================================================
    alphaDialSlider.slider.onValueChange = [this] {alphaDialSlider_onValueChange();};   // アルファダイヤルの操作
    leftButton.onClick = [this] {leftButton_onClick();};    // 左矢印ボタンの処理
    rightButton.onClick = [this] {rightButton_onClick();};  // 右矢印ボタンの処理

    // ==============================================================================
    // 中央ボタンのエリア
    // ==============================================================================
    for (auto* btn : centerButtons)
    {
        if (btn != nullptr)
        {
            btn->onClick = [this, btn] {centerButton_onClick(btn);};
        }
    }

    // ==============================================================================
    // ★ 右ボタンのエリア
    // ==============================================================================
    for (auto* btn : numberButtons)
    {
        if (btn != nullptr)
        {
            btn->onClick = [this, btn] {numberButton_onClick(btn);};
        }
    }
    enterButton.onClick = [this] {enterButton_onClick();};

    setWantsKeyboardFocus(true);
}

MC500_InputComponent::~MC500_InputComponent()
{
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
    // 全体の描画エリア
    auto totalArea = getLocalBounds().reduced (15);

    // ==============================================================================
    // ★ 全体を上下2段に分割（上部20%を液晶、残りをコントローラー群にする）
    // ==============================================================================
    auto lcdBounds = totalArea.removeFromTop (totalArea.getHeight() * 0.20f);
    lcdArea.setBounds (lcdBounds);

    totalArea.removeFromTop (15); // 液晶とボタン群の間隔
    auto area = totalArea;       // コントローラーエリア

    // -------------------------------------------------------------
    // 以下、従来の3ブロック分割計算（コントローラーエリア内で実行）
    // -------------------------------------------------------------
    // 計算しやすいように、全体の幅を「左:中央:右 = 5:4:7」の割合で綺麗にカットします
    int totalUnit = 5 + 4 + 7; // = 16
    int singleWidth = area.getWidth() / totalUnit;

    auto leftArea   = area.removeFromLeft (singleWidth * 5);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto centerArea = area.removeFromLeft (singleWidth * 4);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto rightArea  = area; // 残りが右セクション

    // -------------------------------------------------------------
    // 1. 【左セクション】の配置 (2列4行 Grid)
    // -------------------------------------------------------------
    juce::Grid leftGrid;
    leftGrid.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1))
    };
    for (int i = 0; i < 4; ++i)
        leftGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    leftGrid.columnGap = juce::Grid::Px (6);
    leftGrid.rowGap    = juce::Grid::Px (6);

    auto alphaDial = juce::GridItem(alphaDialSlider).withMargin(juce::GridItem::Margin(10.0f));
    leftGrid.items.add (alphaDial.withArea(1, 1, 4, 3));
    leftGrid.items.add (juce::GridItem(leftButton));
    leftGrid.items.add (juce::GridItem(rightButton));

    leftGrid.performLayout (leftArea);

    // -------------------------------------------------------------
    // 2. 【中央セクション】の配置 (2列4行 Grid)
    // -------------------------------------------------------------
    juce::Grid centerGrid;
    centerGrid.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1))
    };
    for (int i = 0; i < 4; ++i)
        centerGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    centerGrid.columnGap = juce::Grid::Px (6);
    centerGrid.rowGap    = juce::Grid::Px (6);

    for (auto* btn : centerButtons)
    {
        if (btn->getButtonText() == "SHIFT")
        {
            centerGrid.items.add (juce::GridItem (*btn).withArea(4, 1, 5, 3));
        }
        else
        {
            centerGrid.items.add (juce::GridItem (*btn));
        }
    }

    centerGrid.performLayout (centerArea);

    // -------------------------------------------------------------
    // 3. 【右セクション】の配置 (3列4行 Grid)
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
    for (auto* btn : numberButtons)
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
    // アルファダイヤル（無限回転ノブ）の操作 (リセット同期型)
    // ==============================================================================

    // 【右回転】
    if (key.getKeyCode() == juce::KeyPress::upKey)
    {
        double step = juce::ModifierKeys::getCurrentModifiers().isShiftDown() ? 10.0 : 1.0;

        // 現在の値（通常は0.0）にステップを加算してイベントを発火させる
        alphaDialSlider.slider.setValue (alphaDialSlider.slider.getValue() + step, juce::sendNotificationSync);
        return true;
    }

    // 【左回転】
    if (key.getKeyCode() == juce::KeyPress::downKey)
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
    // 中央の配列キーの操作
    // ==============================================================================
    if (key.getKeyCode() == 'r' || key.getKeyCode() == 'R')         // REC
    {
        centerButtons[0]->setToggleState(!centerButtons[0]->getToggleState(), juce::NotificationType::sendNotification);
    }
    else if (key.getKeyCode() == 'p' || key.getKeyCode() == 'P')    // PAUSE
    {
        centerButtons[1]->setToggleState(!centerButtons[1]->getToggleState(), juce::NotificationType::sendNotification);
    }
    else if (key.getKeyCode() == 'f' || key.getKeyCode() == 'F')    // FUNC
    {
        centerButtons[2]->triggerClick();
    }
    else if (key.getKeyCode() == 'm' || key.getKeyCode() == 'M')    // MICRO
    {
        centerButtons[3]->triggerClick();
    }
    else if (key.getKeyCode() == juce::KeyPress::backspaceKey)      // RESET
    {
        centerButtons[4]->triggerClick();
    }
    else if (key.getKeyCode() == juce::KeyPress::escapeKey)         // STOP
    {
        centerButtons[5]->triggerClick();
    }

    // ==============================================================================
    // 右のテンキー配列 (0 〜 9 キー) の操作
    // ==============================================================================
    // キーボードの数字（0〜9）から、numberButtons内のインデックスへ変換するマップ
    // 7->0, 8->1, 9->2, 4->3, 5->4, 6->5, 1->6, 2->7, 3->8, 0->9
    int targetIndex = -1;

    if      (key.getKeyCode() == '7' || key.isKeyCode (juce::KeyPress::numberPad7)) targetIndex = 0;
    else if (key.getKeyCode() == '8' || key.isKeyCode (juce::KeyPress::numberPad8)) targetIndex = 1;
    else if (key.getKeyCode() == '9' || key.isKeyCode (juce::KeyPress::numberPad9)) targetIndex = 2;
    else if (key.getKeyCode() == '4' || key.isKeyCode (juce::KeyPress::numberPad4)) targetIndex = 3;
    else if (key.getKeyCode() == '5' || key.isKeyCode (juce::KeyPress::numberPad5)) targetIndex = 4;
    else if (key.getKeyCode() == '6' || key.isKeyCode (juce::KeyPress::numberPad6)) targetIndex = 5;
    else if (key.getKeyCode() == '1' || key.isKeyCode (juce::KeyPress::numberPad1)) targetIndex = 6;
    else if (key.getKeyCode() == '2' || key.isKeyCode (juce::KeyPress::numberPad2)) targetIndex = 7;
    else if (key.getKeyCode() == '3' || key.isKeyCode (juce::KeyPress::numberPad3)) targetIndex = 8;
    else if (key.getKeyCode() == '0' || key.isKeyCode (juce::KeyPress::numberPad0)) targetIndex = 9;

    // 該当する数字キーが押されていた場合、正しいボタンを発火させる
    if (targetIndex != -1)
    {
        if (targetIndex < numberButtons.size() && numberButtons[targetIndex] != nullptr)
        {
            numberButtons[targetIndex]->triggerClick();
            return true;
        }
    }

    // ENTERキー (キーボードの Enter / Return キー、またはテンキーの Enter)
    if (key.getKeyCode() == juce::KeyPress::returnKey)
    {
        enterButton.triggerClick();
        return true;
    }

    return false;
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
        changeButtonMode(false);    // SHIFTキーONを解除する
    }
}

void MC500_InputComponent::timerCallback()
{
    // 1回だけ実行したいので、即座にタイマーを停止
    stopTimer();

    // 全ての構築が終わったこのタイミングであれば、絶対にアサーションを出さずにフォーカスを取れます
    grabKeyboardFocus();
}

void MC500_InputComponent::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    changeButtonMode(modifiers.isShiftDown());
}

void MC500_InputComponent::focusLost(juce::Component::FocusChangeType cause)
{
    juce::Component::focusLost(cause); // 親クラスの処理を呼ぶ
    changeButtonMode(false);    // SHIFTキーONを解除する
}

void MC500_InputComponent::changeButtonMode(bool isShiftDown)
{
    auto* shiftBtn = centerButtons.getLast();   // SHIFTボタン
    // 現在のボタンの状態と、物理キーの状態が食い違っている場合のみ処理
    if (shiftBtn->getToggleState() != isShiftDown)
    {
        // コードからボタンの状態を強制的に同期させ、onClick を走らせる
        shiftBtn->setToggleState(isShiftDown, juce::NotificationType::sendNotification);
    }
}

 void MC500_InputComponent::alphaDialSlider_onValueChange()
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
     LcdComponent::EditParameter currentEditParameter = lcdArea.getEditParameter();
     switch (currentEditParameter)
     {
         case LcdComponent::EditParameter::None:
             break;
         case LcdComponent::EditParameter::Numerator:
             break;
         case LcdComponent::EditParameter::Denominator:
             break;
         case LcdComponent::EditParameter::Measure:
             break;
         case LcdComponent::EditParameter::Beat:
             break;
         case LcdComponent::EditParameter::Tick:
             break;
         case LcdComponent::EditParameter::StepTime:
         {
             if (intDelta > 0)
             {
                 if (stepTimeIndex + 1 < stepTimeValues.size())
                     ++stepTimeIndex;
             }
             else if (intDelta < 0)
             {
                 if (stepTimeIndex > 0)
                     --stepTimeIndex;
             }
             lcdArea.setStepTime (stepTimeValues[stepTimeIndex]);
             break;
         }
         case LcdComponent::EditParameter::Note:
         {
             int numNew = lcdArea.getNoteNumber() + intDelta;
             lcdArea.setNoteNumber (juce::jlimit (0, 127, numNew));
             break;
         }
         case LcdComponent::EditParameter::Velocity:
         {
             int numNew = lcdArea.getVelocity() + intDelta;
             lcdArea.setVelocity (juce::jlimit (0, 127, numNew));
             break;
         }
         case LcdComponent::EditParameter::GateTime:
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
 }

 void MC500_InputComponent::leftButton_onClick()
 {
     if (!leftArrowLookAndFeel.getDrawMode())
     {
         // キャレットを左へ移動する
         auto param = lcdArea.getEditParameter();

         // モードを左（逆順）に切り替える
         int paramInt = static_cast<int>(param);
         paramInt--;
         if (lcdArea.getEditMode() == LcdComponent::EditMode::TimeSignature)
         {
             if (paramInt < static_cast<int>(LcdComponent::EditParameter::Numerator))
             {
                 paramInt = static_cast<int>(LcdComponent::EditParameter::Denominator);
             }
         }
         else
         {
             if (paramInt < static_cast<int>(LcdComponent::EditParameter::StepTime))
             {
                 paramInt = static_cast<int>(LcdComponent::EditParameter::GateTime);
             }
         }
         lcdArea.setEditParameter(static_cast<LcdComponent::EditParameter>(paramInt));
     }
     else
     {
         // タイを入力する（一つ前の音符に対して）
         if (lcdArea.getEditMode() == LcdComponent::EditMode::Editing)
         {
             std::vector<JuceStepEvent> stepEvents1 = stepData.getStepEvents();
             if (!stepEvents1.empty())
             {
                 JuceStepEvent event = stepEvents1.back();
                 if (isTieOK == true && event.noteNumber >= 0)
                 {
                     int step = lcdArea.getStepTime();
                     event.stepTime += step;
                     event.gateTime += step;
                     lcdArea.addTick(step);
                     stepEvent.measure = lcdArea.getMeasure();
                     stepEvent.beat = lcdArea.getBeat();
                     stepEvent.tick = lcdArea.getTick();
                     stepData.changeStepEvent(event);

                     std::cout << stepData.getStepEvents().back().measure << "-";
                     std::cout << stepData.getStepEvents().back().beat << "-";
                     std::cout << stepData.getStepEvents().back().tick << "[";
                     std::cout << stepData.getStepEvents().back().stepTime << ",";
                     std::cout << stepData.getStepEvents().back().noteNumber << ",";
                     std::cout << stepData.getStepEvents().back().velocity << ",";
                     std::cout << stepData.getStepEvents().back().gateTime << "]" << std::endl;
                 }
             }
         }
     }
     isNoteName = false;
 }

 void MC500_InputComponent::rightButton_onClick()
 {
     if (!rightArrowLookAndFeel.getDrawMode())
     {
         // キャレットを右へ移動する
         auto param = lcdArea.getEditParameter();

         // モードを右（正順）に切り替える
         int paramInt = static_cast<int>(param);
         paramInt++;
         if (lcdArea.getEditMode() == LcdComponent::EditMode::TimeSignature)
         {
             if (paramInt > static_cast<int>(LcdComponent::EditParameter::Denominator))
             {
                 paramInt = static_cast<int>(LcdComponent::EditParameter::Numerator);
             }
         }
         else
         {
             if (paramInt > static_cast<int>(LcdComponent::EditParameter::GateTime))
             {
                 paramInt = static_cast<int>(LcdComponent::EditParameter::StepTime);
             }
         }

         lcdArea.setEditParameter(static_cast<LcdComponent::EditParameter>(paramInt));
     }
     else
     {
         // 休符を入力する
         if (lcdArea.getEditMode() == LcdComponent::EditMode::Editing)
         {
             lcdArea.addTick(lcdArea.getStepTime());
             stepEvent.measure = lcdArea.getMeasure();
             stepEvent.beat = lcdArea.getBeat();
             stepEvent.tick = lcdArea.getTick();
             isTieOK = false;
         }
     }
     isNoteName = false;
 }

 void MC500_InputComponent::centerButton_onClick(juce::TextButton *btn)
 {
     if (btn->getButtonText() == "REC")
     {
         if (btn->getToggleState() == true)
         {
             lcdArea.setEditParameter(LcdComponent::EditParameter::StepTime);
             lcdArea.setEditMode(LcdComponent::EditMode::Editing);
             centerButtons[1]->setClickingTogglesState(true);
         }
         else
         {
             lcdArea.setEditMode(LcdComponent::EditMode::Viewer);
             centerButtons[1]->setClickingTogglesState(false);
             centerButtons[1]->setToggleState (false, juce::dontSendNotification);
         }
     }
     else if (btn->getButtonText() == "PAUSE")
     {
     }
     else if (btn->getButtonText() == "FUNC")
     {
         juce::File smf("./smf.mid");
         stepData.exportToStandardMidiFile(smf);
     }
     else if (btn->getButtonText() == "MICRO")
     {

     }
     else if (btn->getButtonText() == "RESET")
     {
         if (lcdArea.getEditMode() == LcdComponent::EditMode::Editing)
         {
             std::vector<JuceStepEvent> stepEvents1 = stepData.getStepEvents();
             if (!stepEvents1.empty())
             {
                 JuceStepEvent event = stepEvents1.back();
                 if (lcdArea.getMeasure() == event.measure &&
                     lcdArea.getBeat() == event.beat &&
                     lcdArea.getTick() == event.tick &&
                     lcdArea.getStepTime() == event.stepTime &&
                     lcdArea.getNoteNumber() == event.noteNumber &&
                     lcdArea.getVelocity() == event.velocity &&
                     lcdArea.getGateTime() == event.gateTime)
                 {
                     stepData.deleteStepEvent();
                 }

                 stepEvents1 = stepData.getStepEvents();
                 if (!stepEvents1.empty())
                 {
                     // 初期値
                     stepEvent.measure = 1;
                     stepEvent.beat = 1;
                     stepEvent.tick = 0;
                     stepEvent.stepTime = 24;
                     stepEvent.noteNumber = 60;
                     stepEvent.velocity = 64;
                     stepEvent.gateTime = stepEvent.stepTime * 0.75f;    // 長さの75%

                     lcdArea.setMeasure(stepEvent.measure);
                     lcdArea.setBeat(stepEvent.beat);
                     lcdArea.setTick(stepEvent.tick);
                     lcdArea.setStepTime(stepEvent.stepTime);
                     lcdArea.setNoteNumber(-1);
                     lcdArea.setVelocity(-1);
                     lcdArea.setGateTime(-1);
                 }
                 else
                 {
                     stepEvent = event;
                     lcdArea.setMeasure(stepEvent.measure);
                     lcdArea.setBeat(stepEvent.beat);
                     lcdArea.setTick(stepEvent.tick);
                     lcdArea.setStepTime(stepEvent.stepTime);
                     lcdArea.setNoteNumber(stepEvent.noteNumber);
                     lcdArea.setVelocity(stepEvent.velocity);
                     lcdArea.setGateTime(stepEvent.gateTime);
                 }
             }
         }
     }
     else if (btn->getButtonText() == "STOP")
     {

     }
     else if (btn->getButtonText() == "SHIFT")
     {
         if (lcdArea.getEditMode() == LcdComponent::EditMode::Editing)
         {
             bool isShift = btn->getToggleState();
             leftArrowLookAndFeel.setDrawMode(isShift);
             leftButton.repaint();
             rightArrowLookAndFeel.setDrawMode(isShift);
             rightButton.repaint();
             if (isShift)
             {
                 LcdComponent::EditParameter currentEditParameter = lcdArea.getEditParameter();
                 if (currentEditParameter == LcdComponent::EditParameter::StepTime)
                 {
                     for (auto* btnNum : numberButtons)
                     {
                         btnNum->setDrawMode(Multi10KeyDrawMode::NoteDuration);
                     }
                 }
                 else if (currentEditParameter == LcdComponent::EditParameter::Note)
                 {
                     for (auto* btnNum : numberButtons)
                     {
                         btnNum->setDrawMode(Multi10KeyDrawMode::NoteName);
                     }
                 }
             }
             else
             {
                 for (auto* btnNum : numberButtons)
                 {
                     btnNum->setDrawMode(Multi10KeyDrawMode::Number);
                 }
             }
         }
     }
 }

 void MC500_InputComponent::numberButton_onClick(Multi10KeyButton *btn)
 {
     auto* shiftBtn = centerButtons.getLast();   // SHIFTボタン
     int numValue = btn->getNumber();
     LcdComponent::EditParameter currentEditParameter = lcdArea.getEditParameter();
     switch (currentEditParameter)
     {
         case LcdComponent::EditParameter::None:
             break;
         case LcdComponent::EditParameter::Numerator:
             break;
         case LcdComponent::EditParameter::Denominator:
             break;
         case LcdComponent::EditParameter::Measure:
             break;
         case LcdComponent::EditParameter::Beat:
             break;
         case LcdComponent::EditParameter::Tick:
             break;
         case LcdComponent::EditParameter::StepTime:
         {
             if (btn->getDrawMode() == Multi10KeyDrawMode::NoteDuration)
             {
                 int idx = 9 - numValue;
                 if (idx >= 0 && idx < (int)stepTimeValues.size())
                 {
                     lcdArea.setStepTime(stepTimeValues[(size_t)idx]);
                 }
             }
             else
             {
                 int numNew = (lcdArea.getStepTime() * 10 + numValue) % 1000;
                 lcdArea.setStepTime (juce::jlimit (0, 999, numNew));
             }
             break;
         }
         case LcdComponent::EditParameter::Note:
         {
             if(shiftBtn->getToggleState())
             {
                 juce::String inp = btn->getNoteName();
                 bool isNote = std::all_of(inp.begin(), inp.end(), [](char ch) {
                     return ch >= 'A' && ch <= 'G';
                 });
                 juce::String noteNameNew;
                 juce::String noteNameOld = lcdArea.getNoteName();
                 if (isNote)
                 {
                     if (noteNameOld == "REST")
                     {
                         noteNameNew = inp + "   ";
                     }
                     else
                     {
                         noteNameNew = inp + " " + noteNameOld.substring(2);
                     }
                 }
                 else
                 {
                     if (inp == " ")
                     {
                         noteNameNew = "REST";
                     }
                     else
                     {
                         noteNameNew = noteNameOld.substring(0, 1) + inp + noteNameOld.substring(2);
                     }
                 }
                 lcdArea.setNoteName(noteNameNew);
                 isNoteName = true;
             }
             else
             {
                 if (isNoteName)
                 {
                     juce::String noteNameOld = lcdArea.getNoteName();
                     juce::String noteNameNew = noteNameOld.substring(0, 2) + juce::String(numValue);
                     lcdArea.setNoteName(noteNameNew);
                 }
                 else
                 {
                     int numNew = (lcdArea.getNoteNumber() * 10 + numValue) % 1000;
                     lcdArea.setNoteNumber (juce::jlimit (0, 999, numNew));
                 }
             }
             break;
         }
         case LcdComponent::EditParameter::Velocity:
         {
             int numNew = (lcdArea.getVelocity() * 10 + numValue) % 1000;
             lcdArea.setVelocity (juce::jlimit (0, 999, numNew));
             break;
         }
         case LcdComponent::EditParameter::GateTime:
         {
             int numNew = (lcdArea.getGateTime() * 10 + numValue) % 10000;
             lcdArea.setGateTime (juce::jlimit (0, 9999, numNew));
             break;
         }
         default:
             break;
     }
 }

 void MC500_InputComponent::enterButton_onClick()
 {
     auto* shiftBtn = centerButtons.getLast();   // SHIFTボタン
     bool isShift = shiftBtn->getToggleState();
     LcdComponent::EditParameter currentEditParameter = lcdArea.getEditParameter();
     switch (currentEditParameter)
     {
         case LcdComponent::EditParameter::None:
             break;
         case LcdComponent::EditParameter::Numerator:
             stepData.setNumerator(lcdArea.getNumerator());
             lcdArea.setEditParameter(LcdComponent::EditParameter::Denominator);
             break;
         case LcdComponent::EditParameter::Denominator:
             stepData.setDenominator(lcdArea.getDenomirator());
             lcdArea.setEditParameter(LcdComponent::EditParameter::StepTime);
             lcdArea.setEditMode(LcdComponent::EditMode::Editing);
             centerButtons[0]->setToggleState(!centerButtons[0]->getToggleState(), juce::NotificationType::dontSendNotification);
             break;
         case LcdComponent::EditParameter::Measure:
             break;
         case LcdComponent::EditParameter::Beat:
             break;
         case LcdComponent::EditParameter::Tick:
             break;
         case LcdComponent::EditParameter::StepTime:
             stepEvent.stepTime = lcdArea.getStepTime();
             stepEvent.gateTime = stepEvent.stepTime * 0.75f;
             if (lcdArea.getNoteNumber() == -1)
             {
                 lcdArea.setNoteNumber(stepEvent.noteNumber);
             }
             lcdArea.setEditParameter(LcdComponent::EditParameter::Note);
             break;
         case LcdComponent::EditParameter::Note:
             stepEvent.noteNumber = lcdArea.getNoteNumber();
             if (stepEvent.noteNumber == -9)
             {
                 JuceStepEvent stepEvent1 = stepEvent;
                 stepEvent1.velocity = 0;
                 stepEvent1.gateTime = 0;
                 stepData.addStepEvent(stepEvent1);
                 // メジャー・ビート・クロックを進める
                 lcdArea.addTick(lcdArea.getStepTime());
                 stepEvent.measure = lcdArea.getMeasure();
                 stepEvent.beat = lcdArea.getBeat();
                 stepEvent.tick = lcdArea.getTick();

                 isTieOK = false;

                 std::cout << stepData.getStepEvents().back().measure << "-";
                 std::cout << stepData.getStepEvents().back().beat << "-";
                 std::cout << stepData.getStepEvents().back().tick << "[";
                 std::cout << stepData.getStepEvents().back().stepTime << ",";
                 std::cout << stepData.getStepEvents().back().noteNumber << ",";
                 std::cout << stepData.getStepEvents().back().velocity << ",";
                 std::cout << stepData.getStepEvents().back().gateTime << "]" << std::endl;
             }
             else
             {
                 if (lcdArea.getVelocity() == -1)
                 {
                     lcdArea.setVelocity(stepEvent.velocity);
                 }
                 lcdArea.setEditParameter(LcdComponent::EditParameter::Velocity);
             }
             break;
         case LcdComponent::EditParameter::Velocity:
             stepEvent.velocity = lcdArea.getVelocity();
             if (lcdArea.getGateTime() == -1)
             {
                 lcdArea.setGateTime(stepEvent.gateTime);
             }
             lcdArea.setEditParameter(LcdComponent::EditParameter::GateTime);
             break;
         case LcdComponent::EditParameter::GateTime:
             stepEvent.gateTime = lcdArea.getGateTime();
             stepData.addStepEvent(stepEvent);
             if (centerButtons[1]->getToggleState() == isShift)  // PAUSEとSHIFTが同じ状態
             {
                 // メジャー・ビート・クロックを進める
                 lcdArea.addTick(lcdArea.getStepTime());
                 stepEvent.measure = lcdArea.getMeasure();
                 stepEvent.beat = lcdArea.getBeat();
                 stepEvent.tick = lcdArea.getTick();

                 if (isShift == true)    // SHIFTがONのときはPAUSEをOFFにする
                 {
                     centerButtons[1]->triggerClick();
                 }
             }
             // 液晶表示は初期化する
             //lcdArea.setNoteNumber(-1);
             lcdArea.setNoteNumber(stepEvent.noteNumber);
             lcdArea.setVelocity(-1);
             lcdArea.setGateTime(-1);
             lcdArea.setEditParameter(LcdComponent::EditParameter::Note);

             isTieOK = true;

     std::cout << stepData.getStepEvents().back().measure << "-";
     std::cout << stepData.getStepEvents().back().beat << "-";
     std::cout << stepData.getStepEvents().back().tick << "[";
     std::cout << stepData.getStepEvents().back().stepTime << ",";
     std::cout << stepData.getStepEvents().back().noteNumber << ",";
     std::cout << stepData.getStepEvents().back().velocity << ",";
     std::cout << stepData.getStepEvents().back().gateTime << "]" << std::endl;

             break;
         default:
             break;
     }
 }
