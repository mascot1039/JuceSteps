#include "gui.h"

// コンストラクタ：画面が作られたときに最初に動く場所
MainComponent::MainComponent() {
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("IPAGothic");

    // --- 右側・左上・下部の枠設定 ---
    addAndMakeVisible(stepDataDisplay);
    stepDataDisplay.setMultiLine(true);
    stepDataDisplay.setReadOnly(true);
    stepDataDisplay.setCaretVisible(false);
    stepDataDisplay.setScrollbarsShown(true);
    stepDataDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    stepDataDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    stepDataDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain));
    stepDataDisplay.setText(juce::String::fromUTF8("--- MC-50 STEP DATA DISPLAY ---\n"));

    addAndMakeVisible(buttonGroup);
    buttonGroup.setText(juce::String::fromUTF8("MC-50 ボタンエリア"));
    buttonGroup.setVisible(false);

    addAndMakeVisible(inputArea);
    inputArea.setText(juce::String::fromUTF8("入力エリア"));
    inputArea.setVisible(false);

    // --- Auto Forward（自動進み）ボタンの設定 ---
    autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: OFF"));
    autoForwardButton.setClickingTogglesState(true);
    addAndMakeVisible(autoForwardButton);

    autoForwardButton.onClick = [this]() {
        if (autoForwardButton.getToggleState()) {
            autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: *ACTIVE*"));
        } else {
            autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: OFF"));
        }
    };


    // --- 送信ボタンの設定（ステップ確定） ---
    myButton.setButtonText(juce::String::fromUTF8("ステップ確定 (ENTER)"));
    addAndMakeVisible(myButton);
    myButton.onClick = [this]() {
        int meas = (int)measureSlider.getValue();
        int beat = (int)beatSlider.getValue();
        int clk  = (int)clockSlider.getValue();
        int vel  = (int)velocitySlider.getValue();
        int gt   = (int)gateTimeSlider.getValue();
        int st   = (int)stepTimeSlider.getValue();

        juce::String noteText = noteSlider.getTextFromValue(noteSlider.getValue());

        juce::String logLine = juce::String::formatted("[%03d-%02d-%03d] Note:%-4s V:%3d  GT:%3d  ST:%3d\n",
                                                    meas, beat, clk, noteText.toRawUTF8(), vel, gt, st);
        stepDataDisplay.insertTextAtCaret(logLine);

        if (autoForwardButton.getToggleState()) {
            advancePosition(st);
        }
    };


    // --- SKIP（休符）ボタンの設定 ---
    skipButton.setButtonText(juce::String::fromUTF8("SKIP (休符)"));
    addAndMakeVisible(skipButton);
    skipButton.onClick = [this]() {
        int meas = (int)measureSlider.getValue();
        int beat = (int)beatSlider.getValue();
        int clk  = (int)clockSlider.getValue();
        int st   = (int)stepTimeSlider.getValue();

        juce::String logLine = juce::String::formatted("[%03d-%02d-%03d] (Rest) ---------- ST:%3d\n",
                                                    meas, beat, clk, st);
        stepDataDisplay.insertTextAtCaret(logLine);

        advancePosition(st);
    };


    // --- TIE（タイ）ボタンの設定 ---
    tieButton.setButtonText(juce::String::fromUTF8("TIE (タイ)"));
    addAndMakeVisible(tieButton);
    tieButton.onClick = [this]() {
        juce::String allText = stepDataDisplay.getText();
        juce::StringArray lines;
        lines.addLines(allText);

        int lastLineIndex = lines.size() - 1;
        while (lastLineIndex >= 0 && lines[lastLineIndex].trim().isEmpty()) {
            lastLineIndex--;
        }

        if (lastLineIndex >= 0) {
            juce::String lastLine = lines[lastLineIndex];
            if (lastLine.contains("Note:")) {
                int st = (int)stepTimeSlider.getValue();
                int gtIndex = lastLine.indexOf(0, "GT:");
                if (gtIndex >= 0) {
                    int gtStart = gtIndex + 3;
                    int currentGT = lastLine.substring(gtStart, gtStart + 3).trim().getIntValue();
                    int newGT = currentGT + st;

                    juce::String leftPart = lastLine.substring(0, gtStart);
                    int stIndex = lastLine.indexOf(gtStart, "ST:");
                    juce::String rightPart = lastLine.substring(stIndex);

                    juce::String newLine = leftPart + juce::String::formatted("%3d  ", newGT) + rightPart;
                    lines.set(lastLineIndex, newLine);

                    juce::String updatedText = "";
                    for (auto& line : lines) {
                        updatedText += line + "\n";
                    }
                    stepDataDisplay.setText(updatedText);
                }
            }
        }

        advancePosition((int)stepTimeSlider.getValue());
    };


    // --- DELETE（消去）ボタンの設定 ---
    deleteButton.setButtonText(juce::String::fromUTF8("DELETE (消去)"));
    addAndMakeVisible(deleteButton);
    deleteButton.onClick = [this]() {
        juce::String allText = stepDataDisplay.getText();
        juce::StringArray lines;
        lines.addLines(allText);

        int lastLineIndex = lines.size() - 1;
        while (lastLineIndex >= 0 && lines[lastLineIndex].trim().isEmpty()) {
            lastLineIndex--;
        }

        if (lastLineIndex > 0) {
            juce::String lastLine = lines[lastLineIndex];
            int rollbackClocks = 0;

            int stIndex = lastLine.indexOf(0, "ST:");
            if (stIndex >= 0) {
                rollbackClocks = lastLine.substring(stIndex + 3).trim().getIntValue();
            }

            lines.remove(lastLineIndex);

            juce::String updatedText = "";
            for (int i = 0; i <= lines.size() - 1; ++i) {
                if (!lines[i].trim().isEmpty() || i == 0) {
                    updatedText += lines[i] + "\n";
                }
            }
            stepDataDisplay.setText(updatedText);

            if (rollbackClocks > 0) {
                rollbackPosition(rollbackClocks);
            }
        }
    };


    // --- 音符プリセットボタンの設定 ---
    btnNote2.setButtonText(juce::String::fromUTF8("[ 2分音符 ] ST:192"));
    btnNote4.setButtonText(juce::String::fromUTF8("[ 4分音符 ] ST:96"));
    btnNote8.setButtonText(juce::String::fromUTF8("[ 8分音符 ] ST:48"));
    btnNote8T.setButtonText(juce::String::fromUTF8("[ 8分3連  ] ST:32"));
    btnNote16.setButtonText(juce::String::fromUTF8("[16分音符 ] ST:24"));
    btnNote16T.setButtonText(juce::String::fromUTF8("[16分3連 ] ST:16"));
    btnNote32.setButtonText(juce::String::fromUTF8("[32分音符 ] ST:12"));
    btnNote64.setButtonText(juce::String::fromUTF8("[64分音符 ] ST:6"));

    btnNote2.onClick   = [this]() { setStepAndGate(192); };
    btnNote4.onClick   = [this]() { setStepAndGate(96); };
    btnNote8.onClick   = [this]() { setStepAndGate(48); };
    btnNote8T.onClick  = [this]() { setStepAndGate(32); };
    btnNote16.onClick  = [this]() { setStepAndGate(24); };
    btnNote16T.onClick = [this]() { setStepAndGate(16); };
    btnNote32.onClick  = [this]() { setStepAndGate(12); };
    btnNote64.onClick  = [this]() { setStepAndGate(6); };

    addAndMakeVisible(btnNote2);
    addAndMakeVisible(btnNote4);
    addAndMakeVisible(btnNote8);
    addAndMakeVisible(btnNote8T);
    addAndMakeVisible(btnNote16);
    addAndMakeVisible(btnNote16T);
    addAndMakeVisible(btnNote32);
    addAndMakeVisible(btnNote64);

    // テンキーのボタン（計12個）を生成して初期化
    juce::StringArray strLabels = { "",  "<", ">",
                                    "7", "8", "9",
                                    "4", "5", "6",
                                    "1", "2", "3",
                                    "0" };
    for (auto& label : strLabels)
    {
        if (label.isEmpty())
        {
            // 空白の場所には、ボタンではなく「透明な何もないコンポーネント」を置く
            auto* dummy = btn10key.add (new juce::Component());
            addAndMakeVisible(dummy);
        }
        else
        {
            auto* btn = new juce::TextButton(label);
            btn10key.add (btn);
            addAndMakeVisible (btn);

            // ボタンがクリックされたときの処理（例）
            btn->onClick = [label]() { DBG ("Pressed: " + label); };
        }
    }
    // Enterボタンだけは特別なので、個別で作成して addAndMakeVisible する
    addAndMakeVisible(btnEnter);
    btnEnter.setButtonText("Enter");
    btnEnter.onClick = []() { DBG ("Pressed: Enter"); };

    // --- 各入力欄の設定（初期値と範囲） ---
    measureLabel.setText(juce::String::fromUTF8("メジャー"), juce::dontSendNotification);
    beatLabel.setText(juce::String::fromUTF8("ビート"), juce::dontSendNotification);
    clockLabel.setText(juce::String::fromUTF8("クロック"), juce::dontSendNotification);
    noteLabel.setText(juce::String::fromUTF8("ノート"), juce::dontSendNotification);
    velocityLabel.setText(juce::String::fromUTF8("ベロシティ"), juce::dontSendNotification);
    gateTimeLabel.setText(juce::String::fromUTF8("ゲート"), juce::dontSendNotification);
    stepTimeLabel.setText(juce::String::fromUTF8("ステップ"), juce::dontSendNotification);

    measureSlider.setRange(1, 999, 1);
    beatSlider.setRange(1, 4, 1);
    clockSlider.setRange(1, 96, 1);
    noteSlider.setRange(0, 127, 1);
    velocitySlider.setRange(0, 127, 1);
    gateTimeSlider.setRange(1, 9999, 1);
    stepTimeSlider.setRange(1, 9999, 1);

    noteSlider.textFromValueFunction = [](double value) {
        int noteNum = (int)value;
        const char* noteNames[] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
        return juce::String::formatted ("%s%d", noteNames[noteNum % 12], (noteNum / 12) - 1);
    };
    noteSlider.valueFromTextFunction = [](const juce::String& text) { return (double)text.getIntValue(); };

    juce::Slider* sliders[] = { &measureSlider, &beatSlider, &clockSlider, &noteSlider, &velocitySlider, &gateTimeSlider, &stepTimeSlider };
    juce::Label* labels[]   = { &measureLabel, &beatLabel, &clockLabel, &noteLabel, &velocityLabel, &gateTimeLabel, &stepTimeLabel };

    for (int i = 0; i < 7; ++i)
    {
        addAndMakeVisible(labels[i]);

        labels[i]->setJustificationType(juce::Justification::centred);

        addAndMakeVisible(sliders[i]);
        sliders[i]->setSliderStyle(juce::Slider::LinearBar);
        sliders[i]->setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        sliders[i]->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
        sliders[i]->setSliderSnapsToMousePosition(false);
    }

    // 初期値を安全にセット
    measureSlider.setValue(1);
    beatSlider.setValue(1);
    clockSlider.setValue(1);
    noteSlider.setValue(60);
    velocitySlider.setValue(100);
    gateTimeSlider.setValue(24);
    stepTimeSlider.setValue(24);

    setWantsKeyboardFocus(true);
    setSize(960, 600);
}

// 画面を描く場所
void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

// 画面のサイズが変わったときに動く場所（今回は空っぽでOK）
void MainComponent::resized() {
    auto bounds = getLocalBounds().reduced(5);
    const int gap = 5;

    // 1. 【右側】 Display STEP data エリア（40%）
    auto rightWidth = bounds.getWidth() * 0.4;
    stepDataDisplay.setBounds(bounds.removeFromRight(rightWidth));
    bounds.removeFromRight(gap);

    // 2. 【下部】 入力エリア（30%）
    auto bottomHeight = bounds.getHeight() * 0.3;
    auto inputBounds = bounds.removeFromBottom(bottomHeight);
    inputArea.setBounds(inputBounds);

    auto contentBounds = inputBounds.reduced(10, 5).withTrimmedTop(15);
    int itemWidth = contentBounds.getWidth() / 7;

    juce::Slider* sliders[] = { &measureSlider, &beatSlider, &clockSlider, &noteSlider, &velocitySlider, &gateTimeSlider, &stepTimeSlider };
    juce::Label* labels[]   = { &measureLabel, &beatLabel, &clockLabel, &noteLabel, &velocityLabel, &gateTimeLabel, &stepTimeLabel };

    for (int i = 0; i < 7; ++i)
    {
        auto itemArea = contentBounds.removeFromLeft(itemWidth);
        labels[i]->setBounds(itemArea.removeFromTop(itemArea.getHeight() * 0.4));
        sliders[i]->setBounds(itemArea.reduced(2, 2));
    }

    // 3. 【左上】 MC-50 button エリア
    bounds.removeFromBottom(gap);
    buttonGroup.setBounds(bounds);

    auto buttonArea = bounds.reduced(15, 10).withTrimmedTop(25);

    int columnWidth = (buttonArea.getWidth() - gap) / 2;
    auto leftColumn = buttonArea.removeFromLeft(columnWidth);
    buttonArea.removeFromLeft(gap);
    auto rightColumn = buttonArea;

    // 【左側の列】 操作系ボタン
    //myButton.setBounds(leftColumn.removeFromTop(28));
    //leftColumn.removeFromTop(gap);
    //autoForwardButton.setBounds(leftColumn.removeFromTop(28));
    //leftColumn.removeFromTop(gap);
    //skipButton.setBounds(leftColumn.removeFromTop(28));
    //leftColumn.removeFromTop(gap);
    //tieButton.setBounds(leftColumn.removeFromTop(28));
    //leftColumn.removeFromTop(gap);
    //deleteButton.setBounds(leftColumn.removeFromTop(28));

    //// 【右側の列】 音符プリセットエリア
    //int subColumnWidth = (rightColumn.getWidth() - gap) / 2;
    //auto noteLeftSubColumn = rightColumn.removeFromLeft(subColumnWidth);
    ////rightColumn.removeFromLeft(gap);
    ////auto noteRightSubColumn = rightColumn;

    //btnNote2.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote4.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote8.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote8T.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote16.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote16T.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote32.setBounds(noteLeftSubColumn.removeFromTop(35));
    //noteLeftSubColumn.removeFromTop(gap);
    //btnNote64.setBounds(noteLeftSubColumn.removeFromTop(35));

    juce::Grid grid;

    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    // 3列 × 5行 のグリッドを等倍（1Fr）で定義
    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
    grid.templateRows = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    // 隙間（ギャップ）の設定
    grid.setGap(juce::Grid::Px(5));

    // すべてのボタンをグリッドアイテムとして追加
    for (auto *btn : btn10key)
      grid.items.add(juce::GridItem(btn));

    // ★ Enterボタンの配置を「ピンポイント指定」してグリッドへ追加
    // withArea ( 開始行, 開始列, 終了行+1, 終了列+1 ) というルールで指定します
    grid.items.add (juce::GridItem (btnEnter).withArea(5, 2, 6, 4));

    // コンポーネントの領域全体にグリッドを適用
    // grid.performLayout (getLocalBounds().reduced (10)); // 周囲に10pxの余白
    grid.performLayout(buttonArea);
}

bool MainComponent::keyPressed (const juce::KeyPress& key) {
    if (key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey)
    {
        if (deleteButton.onClick != nullptr) { deleteButton.onClick(); return true; }
    }
    else if (key == juce::KeyPress::returnKey)
    {
        if (myButton.onClick != nullptr) { myButton.onClick(); return true; }
    }
    else if (key == juce::KeyPress::spaceKey)
    {
        if (skipButton.onClick != nullptr) { skipButton.onClick(); return true; }
    }
    else if (key.getKeyCode() == 'T' || key.getKeyCode() == 't')
    {
        if (tieButton.onClick != nullptr) { tieButton.onClick(); return true; }
    }
    return false;
}



// --- 【重要】コンパイルエラーを解消する位置計算・プリセット関数群 ---
void MainComponent::advancePosition (int clocksToAdd)
{
    int meas = (int)measureSlider.getValue();
    int beat = (int)beatSlider.getValue();
    int clk  = (int)clockSlider.getValue();

    long currentTotalClocks = ((meas - 1) * 384) + ((beat - 1) * 96) + (clk - 1);
    long nextTotalClocks = currentTotalClocks + clocksToAdd;

    int nextMeas = (int)(nextTotalClocks / 384) + 1;
    int remain   = (int)(nextTotalClocks % 384);
    int nextBeat = (remain / 96) + 1;
    int nextClk  = (remain % 96) + 1;

    measureSlider.setValue (nextMeas);
    beatSlider.setValue (nextBeat);
    clockSlider.setValue (nextClk);
}

void MainComponent::rollbackPosition (int clocksToRemove)
{
    int meas = (int)measureSlider.getValue();
    int beat = (int)beatSlider.getValue();
    int clk  = (int)clockSlider.getValue();

    long currentTotalClocks = ((meas - 1) * 384) + ((beat - 1) * 96) + (clk - 1);
    long prevTotalClocks = currentTotalClocks - clocksToRemove;

    if (prevTotalClocks < 0) prevTotalClocks = 0;

    int nextMeas = (int)(prevTotalClocks / 384) + 1;
    int remain   = (int)(prevTotalClocks % 384);
    int nextBeat = (remain / 96) + 1;
    int nextClk  = (remain % 96) + 1;

    measureSlider.setValue (nextMeas);
    beatSlider.setValue (nextBeat);
    clockSlider.setValue (nextClk);
}

void MainComponent::setStepAndGate (int clockValue)
{
    stepTimeSlider.setValue (clockValue);
    gateTimeSlider.setValue (clockValue);
}
