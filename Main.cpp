#include <juce_gui_basics/juce_gui_basics.h>

// ==============================================================================
// 1. メイン画面のコンポーネント (MainComponent)
// ==============================================================================
class MainComponent : public juce::Component {
public:
    MainComponent() {
        // 日本語フォント（ゴシック体）の設定
        juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("IPAGothic");

        // --- 右側：ディスプレイエリアの設定 ---
        addAndMakeVisible(stepDataDisplay);
        stepDataDisplay.setMultiLine(true);
        stepDataDisplay.setReadOnly(true);
        stepDataDisplay.setCaretVisible(false);
        stepDataDisplay.setScrollbarsShown(true);
        stepDataDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
        stepDataDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
        stepDataDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain));
        stepDataDisplay.setText(juce::String::fromUTF8("--- MC-50 STEP DATA DISPLAY ---\n"));

        // --- 各グループ枠の設定 ---
        addAndMakeVisible(buttonGroup);
        buttonGroup.setText(juce::String::fromUTF8("MC-50 ボタンエリア"));

        addAndMakeVisible(inputArea);
        inputArea.setText(juce::String::fromUTF8("入力エリア"));


        // --- ボタンの設定：1. ステップ確定 (ENTER) ---
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

            // ディスプレイへデータを送信
            juce::String logLine = juce::String::formatted("[%03d-%02d-%03d] Note:%-4s V:%3d  GT:%3d  ST:%3d\n",
                                                        meas, beat, clk, noteText.toRawUTF8(), vel, gt, st);
            stepDataDisplay.insertTextAtCaret(logLine);

            // Auto Forward が ON なら、ステップタイム分だけ時間を進める
            if (autoForwardButton.getToggleState()) {
                advanceTime(st);
            }
        };


        // --- ボタンの設定：2. Auto Forward (ON/OFF) ---
        autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: OFF"));
        autoForwardButton.setClickingTogglesState(true); // 押すたびにON/OFFが切り替わる設定
        addAndMakeVisible(autoForwardButton);
        autoForwardButton.onClick = [this]() {
            if (autoForwardButton.getToggleState()) {
                autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: ON"));
            } else {
                autoForwardButton.setButtonText(juce::String::fromUTF8("Auto Forward: OFF"));
            }
        };


        // --- ボタンの設定：3. SKIP (休符) ---
        skipButton.setButtonText(juce::String::fromUTF8("SKIP (休符)"));
        addAndMakeVisible(skipButton);
        skipButton.onClick = [this]() {
            int meas = (int)measureSlider.getValue();
            int beat = (int)beatSlider.getValue();
            int clk  = (int)clockSlider.getValue();
            int st   = (int)stepTimeSlider.getValue();

            // ディスプレイに休符（Rest）として記録を流す
            juce::String logLine = juce::String::formatted("[%03d-%02d-%03d] (Rest) ---------- ST:%3d\n",
                                                        meas, beat, clk, st);
            stepDataDisplay.insertTextAtCaret(logLine);

            // 休符なので音は鳴らさず、時間だけをステップタイム分進める
            advanceTime(st);
        };


        // --- TIE（タイ）ボタンの設定 ---
        tieButton.setButtonText(juce::String::fromUTF8("TIE (タイ)"));
        addAndMakeVisible(tieButton);

        tieButton.onClick = [this]() {
            // 1. 直前の入力データを解析して、ゲートタイム（GT）を伸ばす処理
            juce::String allText = stepDataDisplay.getText();

            juce::StringArray lines;
            lines.addLines(allText);

            int lastLineIndex = lines.size() - 1;
            while (lastLineIndex >= 0 && lines[lastLineIndex].trim().isEmpty()) {
                lastLineIndex--;
            }

            if (lastLineIndex >= 0)
            {
                juce::String lastLine = lines[lastLineIndex];

                if (lastLine.contains("Note:"))
                {
                    int st = (int)stepTimeSlider.getValue();

                    // 「GT:」の位置を正しく検索 (開始位置, 探す文字列)
                    int gtIndex = lastLine.indexOf(0, "GT:");

                    if (gtIndex >= 0)
                    {
                        // 「GT:」の直後の3文字を切り出して数値にする
                        int gtStart = gtIndex + 3;
                        int currentGT = lastLine.substring(gtStart, gtStart + 3).trim().getIntValue();

                        // ゲートタイムを現在のSTの分だけ伸ばす！
                        int newGT = currentGT + st;

                        // 文字列の「GT:」の手前と、「ST:」以降を綺麗に分割する
                        juce::String leftPart = lastLine.substring(0, gtStart);

                        int stIndex = lastLine.indexOf(gtStart, "ST:");
                        juce::String rightPart = lastLine.substring(stIndex);

                        // 「GT:」の数値を3桁の幅で埋め直して合体
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

            // 2. 時間（位置）をステップタイム分だけ進める（確定やSKIPと同じ計算）
            int meas = (int)measureSlider.getValue();
            int beat = (int)beatSlider.getValue();
            int clk  = (int)clockSlider.getValue();
            int st   = (int)stepTimeSlider.getValue();

            long currentTotalClocks = ((meas - 1) * 384) + ((beat - 1) * 96) + (clk - 1);
            long nextTotalClocks = currentTotalClocks + st;

            int nextMeas = (int)(nextTotalClocks / 384) + 1;
            int remain   = (int)(nextTotalClocks % 384);
            int nextBeat = (remain / 96) + 1;
            int nextClk  = (remain % 96) + 1;

            measureSlider.setValue(nextMeas);
            beatSlider.setValue(nextBeat);
            clockSlider.setValue(nextClk);
        };


        // --- DELETE（巻き戻し）ボタンの設定 ---
        deleteButton.setButtonText(juce::String::fromUTF8("DELETE (消去)"));
        addAndMakeVisible(deleteButton);

        deleteButton.onClick = [this]() {
            juce::String allText = stepDataDisplay.getText();

            juce::StringArray lines;
            lines.addLines(allText);

            // 空行を除外して、実際にデータが入っている最後の行を探す
            int lastLineIndex = lines.size() - 1;
            while (lastLineIndex >= 0 && lines[lastLineIndex].trim().isEmpty()) {
                lastLineIndex--;
            }

            // タイトル行（--- MC-50 STEP DATA DISPLAY ---）は消さないようにする
            if (lastLineIndex > 0)
            {
                juce::String lastLine = lines[lastLineIndex];
                int rollbackClocks = 0;

                // 消す行から「ST:」の位置を探して、ステップタイムを読み取る
                int stIndex = lastLine.indexOf(0, "ST:");
                if (stIndex >= 0)
                {
                    rollbackClocks = lastLine.substring(stIndex + 3).trim().getIntValue();
                }

                // 1. 最後の1行を削除してディスプレイを更新
                lines.remove(lastLineIndex);

                juce::String updatedText = "";
                for (int i = 0; i <= lines.size() - 1; ++i) {
                    if (!lines[i].trim().isEmpty() || i == 0) {
                        updatedText += lines[i] + "\n";
                    }
                }
                stepDataDisplay.setText(updatedText);

                // 2. 読み取ったステップタイムの分だけ、時間を「巻き戻す」計算
                if (rollbackClocks > 0)
                {
                    int meas = (int)measureSlider.getValue();
                    int beat = (int)beatSlider.getValue();
                    int clk  = (int)clockSlider.getValue();

                    // 現在位置を通算クロックに変換
                    long currentTotalClocks = ((meas - 1) * 384) + ((beat - 1) * 96) + (clk - 1);

                    // ステップタイム分だけ引き算する
                    long prevTotalClocks = currentTotalClocks - rollbackClocks;

                    // 1小節目の頭（0クロック）より前には戻らないようにガード
                    if (prevTotalClocks < 0) {
                        prevTotalClocks = 0;
                    }

                    // 新しい「小節・拍・クロック」を逆算してスライダーにセット
                    int nextMeas = (int)(prevTotalClocks / 384) + 1;
                    int remain   = (int)(prevTotalClocks % 384);
                    int nextBeat = (remain / 96) + 1;
                    int nextClk  = (remain % 96) + 1;

                    measureSlider.setValue(nextMeas);
                    beatSlider.setValue(nextBeat);
                    clockSlider.setValue(nextClk);
                }
            }
        };


        // --- 【修正版】音符プリセットボタンの設定 ---
        btnNote4.setButtonText(juce::String::fromUTF8("[ 4分音符 ]  ST:96"));
        btnNote8.setButtonText(juce::String::fromUTF8("[ 8分音符 ]  ST:48"));
        btnNote16.setButtonText(juce::String::fromUTF8("[16分音符 ]  ST:24"));
        btnNote32.setButtonText(juce::String::fromUTF8("[32分音符 ]  ST:12"));

        // 各ボタンが押されたら、ステップとゲートの値を一発で変更するラムダ関数
        btnNote4.onClick  = [this]() { stepTimeSlider.setValue(96); gateTimeSlider.setValue(96); };
        btnNote8.onClick  = [this]() { stepTimeSlider.setValue(48); gateTimeSlider.setValue(48); };
        btnNote16.onClick = [this]() { stepTimeSlider.setValue(24); gateTimeSlider.setValue(24); };
        btnNote32.onClick = [this]() { stepTimeSlider.setValue(12); gateTimeSlider.setValue(12); };

        addAndMakeVisible(btnNote4);
        addAndMakeVisible(btnNote8);
        addAndMakeVisible(btnNote16);
        addAndMakeVisible(btnNote32);


        // --- 各入力欄の設定（初期値と範囲） ---
        measureLabel.setText(juce::String::fromUTF8("メジャー"), juce::dontSendNotification);
        beatLabel.setText(juce::String::fromUTF8("ビート"), juce::dontSendNotification);
        clockLabel.setText(juce::String::fromUTF8("クロック"), juce::dontSendNotification);
        noteLabel.setText(juce::String::fromUTF8("ノート"), juce::dontSendNotification);
        velocityLabel.setText(juce::String::fromUTF8("ベロシティ"), juce::dontSendNotification);
        gateTimeLabel.setText(juce::String::fromUTF8("ゲート"), juce::dontSendNotification);
        stepTimeLabel.setText(juce::String::fromUTF8("ステップ"), juce::dontSendNotification);

        measureSlider.setRange(1, 999, 1);   // 1〜999小節
        beatSlider.setRange(1, 4, 1);        // 1〜4拍（4/4拍子想定）
        clockSlider.setRange(1, 96, 1);      // 1〜96クロック（1拍=96クロック）
        noteSlider.setRange(0, 127, 1);
        velocitySlider.setRange(0, 127, 1);
        gateTimeSlider.setRange(1, 9999, 1);
        stepTimeSlider.setRange(1, 9999, 1);

        // ノート番号(0-127)をMIDI音名（C4など）に変換するルール
        noteSlider.textFromValueFunction = [](double value) {
            int noteNum = (int)value;
            const char* noteNames[] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
            return juce::String::formatted ("%s%d", noteNames[noteNum % 12], (noteNum / 12) - 1);
        };
        noteSlider.valueFromTextFunction = [](const juce::String& text) { return (double)text.getIntValue(); };

        // 7つの入力項目を一括で画面に登録＆スタイル設定
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

        // 初期値の安全なセット（16分音符=24クロックを基準に設定）
        measureSlider.setValue(1);
        beatSlider.setValue(1);
        clockSlider.setValue(1);
        noteSlider.setValue(60); // 真ん中のC (C4)
        velocitySlider.setValue(100);
        gateTimeSlider.setValue(24);
        stepTimeSlider.setValue(24);

        // --- 【新機能】コンポーネントがキーボードのフォーカスを受け取れるようにする ---
        setWantsKeyboardFocus(true);

        setSize(800, 600);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(5);
        const int gap = 5;

        // 1. 【右側】 ディスプレイエリア (幅40%)
        auto rightWidth = bounds.getWidth() * 0.4;
        stepDataDisplay.setBounds(bounds.removeFromRight(rightWidth));
        bounds.removeFromRight(gap);

        // 2. 【下部】 入力エリア (高さ20%)
        auto bottomHeight = bounds.getHeight() * 0.2;
        auto inputBounds = bounds.removeFromBottom(bottomHeight);
        inputArea.setBounds(inputBounds);

        // 入力エリアの中身を7等分して横に並べる
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

        // ボタンの配置エリア全体（内側の余白をとる）
        auto buttonArea = bounds.reduced(15, 10).withTrimmedTop(25);

        // 領域を左右「2つの列」に分割する
        int columnWidth = (buttonArea.getWidth() - gap) / 2;
        auto leftColumn = buttonArea.removeFromLeft(columnWidth);
        buttonArea.removeFromLeft(gap); // 隙間
        auto rightColumn = buttonArea;   // 残った右側の列

        // 【左側の列】 これまでの操作系ボタン（確定、Auto Fwd、SKIP、TIE、DELETE）
        myButton.setBounds(leftColumn.removeFromTop(30));
        leftColumn.removeFromTop(gap);
        autoForwardButton.setBounds(leftColumn.removeFromTop(30));
        leftColumn.removeFromTop(gap);
        skipButton.setBounds(leftColumn.removeFromTop(30));
        leftColumn.removeFromTop(gap);
        tieButton.setBounds(leftColumn.removeFromTop(30));
        leftColumn.removeFromTop(gap);
        deleteButton.setBounds(leftColumn.removeFromTop(30));

        // 【右側の列】 新しい音符プリセットボタン
        btnNote4.setBounds(rightColumn.removeFromTop(35));
        rightColumn.removeFromTop(gap);
        btnNote8.setBounds(rightColumn.removeFromTop(35));
        rightColumn.removeFromTop(gap);
        btnNote16.setBounds(rightColumn.removeFromTop(35));
        rightColumn.removeFromTop(gap);
        btnNote32.setBounds(rightColumn.removeFromTop(35));
    }

    // --- 【新機能】パソコンのキーボードが押されたときに呼び出される関数 ---
    bool keyPressed (const juce::KeyPress& key) override
    {
        // 1. Backspaceキー または Deleteキー が押されたとき
        if (key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey)
        {
            if (deleteButton.onClick != nullptr) {
                deleteButton.onClick(); // DELETEボタンの処理を実行！
                return true;            // キー入力を処理したことをJUCEに伝える
            }
        }
        // 2. Enterキー（テンキーのEnterも含む）が押されたとき
        else if (key == juce::KeyPress::returnKey)
        {
            if (myButton.onClick != nullptr) {
                myButton.onClick();     // 確定ボタンの処理を実行！
                return true;
            }
        }
        // 3. Spaceキー が押されたとき
        else if (key == juce::KeyPress::spaceKey)
        {
            if (skipButton.onClick != nullptr) {
                skipButton.onClick();   // SKIPボタンの処理を実行！
                return true;
            }
        }
        // 4. ★【新機能】「T」または「t」キー が押されたとき
        else if (key.getKeyCode() == 'T' || key.getKeyCode() == 't')
        {
            if (tieButton.onClick != nullptr) {
                tieButton.onClick();    // TIEボタンの処理を実行！
                return true;
            }
        }

        return false; // 使わないキーはスルーする
    }

private:
    // --- 【共通処理】ステップタイムの分だけ時間を進める計算メソッド ---
    void advanceTime(int stepTimeClocks) {
        int meas = (int)measureSlider.getValue();
        int beat = (int)beatSlider.getValue();
        int clk  = (int)clockSlider.getValue();

        // すべてを一度通算クロックに変換（1小節=384, 1拍=96クロック）
        long currentTotalClocks = ((meas - 1) * 384) + ((beat - 1) * 96) + (clk - 1);
        long nextTotalClocks = currentTotalClocks + stepTimeClocks;

        // 新しい 小節・拍・クロック を逆算
        int nextMeas = (int)(nextTotalClocks / 384) + 1;
        int remain   = (int)(nextTotalClocks % 384);
        int nextBeat = (remain / 96) + 1;
        int nextClk  = (remain % 96) + 1;

        // 画面の数値を更新
        measureSlider.setValue(nextMeas);
        beatSlider.setValue(nextBeat);
        clockSlider.setValue(nextClk);
    }

    // 画面の大枠
    juce::TextEditor stepDataDisplay;
    juce::GroupComponent buttonGroup;
    juce::GroupComponent inputArea;

    // ボタン類
    juce::TextButton myButton;
    juce::TextButton autoForwardButton;
    juce::TextButton skipButton;
    juce::TextButton tieButton;
    juce::TextButton deleteButton;

    // --- 【新機能】音符プリセットボタンの変数群 ---
    juce::TextButton btnNote4;  // 4分音符 (96)
    juce::TextButton btnNote8;  // 8分音符 (48)
    juce::TextButton btnNote16; // 16分音符 (24)
    juce::TextButton btnNote32; // 32分音符 (12)

    // 入力アイテム群（スライダー・ラベル）
    juce::Slider measureSlider;
    juce::Slider beatSlider;
    juce::Slider clockSlider;
    juce::Slider noteSlider;
    juce::Slider velocitySlider;
    juce::Slider gateTimeSlider;
    juce::Slider stepTimeSlider;

    juce::Label measureLabel;
    juce::Label beatLabel;
    juce::Label clockLabel;
    juce::Label noteLabel;
    juce::Label velocityLabel;
    juce::Label gateTimeLabel;
    juce::Label stepTimeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

// ==============================================================================
// 2. JUCE アプリケーションクラス (Application)
// ==============================================================================
class Application : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION; }
    void initialise(const juce::String&) override {
        window = std::make_unique<MainWindow>(getApplicationName());
    }
    void shutdown() override { window = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        // ==============================================================================
        // 3. メインウィンドウクラス (MainWindow)
        // ==============================================================================
        MainWindow(const juce::String& name)
            : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), allButtons) {
            setContentOwned(new MainComponent(), true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    };
    std::unique_ptr<MainWindow> window;
};

// アプリケーションの起動マクロ
START_JUCE_APPLICATION(Application)
