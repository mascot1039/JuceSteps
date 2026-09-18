#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "InfiniteRotarySliderComponent.h"
#include "LcdComponent.h"

// ==============================================================================
// ★実機風の黒い丸型ダイヤルを描画するためのカスタムLook&Feelクラス
// ==============================================================================
class DialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DialLookAndFeel() {}

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        // 描画エリアの計算（正方形の中心をとる）
        auto radius = juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;

        // ==============================================================================
        // ★【修正】つまみ本体の色を、周りのボタンと同じ色（デフォルトグレー）にする
        // ==============================================================================
        auto buttonColor = slider.findColour (juce::TextButton::buttonColourId);
        g.setColour (buttonColor);
        g.fillEllipse (rx, ry, rw, rw);

        // 2. 外周の立体感を出す境界線（ベベル効果）
        // つまみが明るくなったので、影（黒）とハイライト（白）の不透明度を調整して馴染ませます
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.drawEllipse (rx + 1.0f, ry + 1.0f, rw - 2.0f, rw - 2.0f, 1.0f);

        // 3. 回転角度の計算
        auto currentAngle = rotaryStartAngle + (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));

        // 4. インジケーター（指標線）の描画
        // つまみがグレーになったため、指標線は「黒（または濃いグレー）」にすると見やすくなります
        juce::Path p;
        auto pointerLength = radius * 0.8f;
        auto pointerThickness = 3.0f;

        p.addRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, radius * 0.25f);
        p.applyTransform (juce::AffineTransform::rotation (currentAngle).translated (centreX, centreY));

        g.setColour(juce::Colour (0xFFECEAE4));
        //g.setColour (juce::Colour (0xFF2A2A2A)); // ★ 指標線を濃いグレーに変更して視認性を確保
        g.fillPath (p);
    }
};

// ==============================================================================
// メインコンポーネントクラス
// ==============================================================================
class MC500_InputComponent  : public juce::Component
{
public:
    MC500_InputComponent();
    ~MC500_InputComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // --- UI素材の定義 ---
    LcdComponent lcdArea;

    InfiniteRotarySliderComponent alphaDialSlider;
    juce::TextButton tieButton;
    juce::TextButton restButton;

    // 中央のボタン配列 (OwnedArrayはスマートポインタの動的配列)
    juce::OwnedArray<juce::TextButton> centerButtons;

    // 右のテンキー配列
    juce::OwnedArray<juce::TextButton> numButtons;
    juce::TextButton zeroButton;
    juce::TextButton enterButton;

    // ★カスタムLook&Feelのインスタンスを追加
    DialLookAndFeel dialLookAndFeel;

    void setupButton (juce::TextButton& btn, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MC500_InputComponent)
};
