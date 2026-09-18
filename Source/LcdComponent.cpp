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
    // 1. MC-50風の液晶背景色（少し緑がかった暗いグレー、またはバックライト風の緑）
    // 当時の雰囲気に合わせて「少し光っている緑色の液晶」をイメージしています
    g.fillAll(juce::Colour::fromRGB(15, 30, 20)); // バックライト消灯風
    // もしバックライト点灯風にするならこちら： g.fillAll(juce::Colour::fromRGB(120, 150, 110));

    // 2. 枠線を描く
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 3);

    // 3. 文字の設定（等幅フォントを使うと数字の桁がズレなくて綺麗です）
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 24.0f, juce::Font::plain));
    g.setColour(juce::Colour::fromRGB(50, 220, 100)); // 黄緑色の文字（自発光風）
    // バックライト点灯風の場合は文字を黒に： g.setColour(juce::Colours::black);

    // 4. 表示する文字列を組み立てる (C++の標準機能でゼロ埋め)
    // 「000-00-000 xxx(000) 000 000 0000」の形式
    juce::String text = juce::String::formatted(
        "%03d-%02d-%03d  %s(%03d)  %03d  %03d  %04d",
        mMeasure, mBeat, mClock,
        mNoteName.toRawUTF8(), mNoteNumber,
        mVelocity,
        mGateTime,
        mStepTime
    );

    // 5. 液晶の少し内側に文字を描画する
    g.drawText(text, getLocalBounds().reduced(20), juce::Justification::topLeft, true);
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
