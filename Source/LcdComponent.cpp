#include "LcdComponent.h"
#include "juce_graphics/juce_graphics.h"

// ==========================================
// LcdComponent の実装
// ==========================================
LcdComponent::LcdComponent() {
    // 液晶画面の初期サイズ（必要に応じて変更してください）
    //setSize(600, 800);
}

void LcdComponent::paint(juce::Graphics& g) {
    // 1. 背景色
    g.fillAll(juce::Colour::fromRGB(15, 30, 20));

    // 2. 枠線
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 3);

    // 3. 文字の設定（等幅フォント）
    float fontSize = 24.0f;
    juce::Font font(juce::Font::getDefaultMonospacedFontName(), fontSize, juce::Font::plain);
    g.setFont(font);

    // 4. 文字列の組み立て
    // フォーマット：「000-00-000  C 3(060)  064  0096  0096」
    juce::String text = juce::String::formatted(
        "%03d-%02d-%03d  %s(%03d)  %03d  %04d  %04d",
        mMeasure, mBeat, mClock,
        mNoteName.toRawUTF8(), mNoteNumber,
        mVelocity,
        mGateTime,
        mStepTime
    );

    // 🌟 描画位置の基準を明確にする
    // reduced(18) と同じ左上座標を取得
    auto textBounds = getLocalBounds().reduced(18);
    float startX = static_cast<float>(textBounds.getX());
    float startY = static_cast<float>(textBounds.getY());

    // フォントから1文字の正確な横幅（アドバンス幅）を取得する
    float charWidth = font.getStringWidthFloat ("A");
    float fontHeight = font.getHeight();

    // 5. 選択項目のハイライト（反転四角形）の座標計算
    float highlightX = startX - 24.0f;
    float highlightW = 0.0f;
    float highlightY = startY - 2.0f; // 文字の高さに合わせる微調整
    float highlightH = fontHeight + 4.0f;

    if (mCurrentMode != EditMode::None)
    {
        switch (mCurrentMode)
        {
            case EditMode::Velocity:
                highlightX += charWidth * 24.0f; // Velocity「064」の開始文字位置
                highlightW = charWidth * 3.0f;   // 3文字分
                break;
            case EditMode::GateTime:
                highlightX += charWidth * 29.0f; // GateTime「0096」の開始文字位置
                highlightW = charWidth * 4.0f;   // 4文字分
                break;
            case EditMode::StepTime:
                highlightX += charWidth * 35.0f; // StepTime「0096」の開始文字位置
                highlightW = charWidth * 4.0f;   // 4文字分
                break;
            default:
                break;
        }

        // 先に背景に黄緑色の四角形を描画
        if (highlightW > 0.0f)
        {
            g.setColour(juce::Colour::fromRGB(50, 220, 100));
            g.fillRect(highlightX, highlightY, highlightW, highlightH);
        }
    }

    // 6. 通常の文字を描画（ベースとなる鮮やかな黄緑色）
    g.setColour(juce::Colour::fromRGB(50, 220, 100));
    g.drawText(text, textBounds, juce::Justification::topLeft, true);

    // 7. 【重要・修正】反転部分の中央の文字だけを「黒（暗いグレー）」でくり抜く処理
    if (mCurrentMode != EditMode::None && highlightW > 0.0f)
    {
        g.saveState();

        // 🌟 グラフィックスの描画エリアを「黄緑色の四角形の中だけ」に制限する（これで他が暗くなりません）
        g.reduceClipRegion (juce::Rectangle<float> (highlightX, highlightY, highlightW, highlightH).getSmallestIntegerContainer());

        // 文字列の色を「背景の暗いグレー」にして、制限エリア内（四角形の中）だけを上書き描画
        g.setColour(juce::Colour::fromRGB(15, 30, 20));
        g.drawText(text, textBounds, juce::Justification::topLeft, true);

        g.restoreState();
    }
}

// 外部から値を変更するための関数（値が変わったら repaint() で画面を再描画する）
void LcdComponent::setTimePosition(int measure, int beat, int clock) {
    mMeasure = measure; mBeat = beat; mClock = clock;
    repaint();
}
void LcdComponent::setNoteInfo(const juce::String& name, int number) {
    mNoteName = name; mNoteNumber = number;
    repaint();
}
void LcdComponent::setVelocity(int vel) {
    mVelocity = vel;
    repaint();
}
void LcdComponent::setGateTime(int gate) {
    mGateTime = gate;
    repaint();
}
void LcdComponent::setStepTime(int step) {
    mStepTime = step;
    repaint();
}

void LcdComponent::setEditMode (EditMode mode) {
    mCurrentMode = mode;
    repaint(); // モードが変わったら液晶を再描画して反転表示を更新
}
