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

    void setVisualAngle (float newAngle) { visualAngle = newAngle; }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        juce::ignoreUnused(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);

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
        //auto currentAngle = rotaryStartAngle + (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));
        auto currentAngle = visualAngle;

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

private:
    float visualAngle = 0.0f; // ▽▽▽ 追記：見た目の回転角度を保持する変数 ▽▽▽
};

// ==============================================================================
// ★左右の矢印ボタン専用のカスタムLook&Feelクラス
// ==============================================================================
class ArrowButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArrowButtonLookAndFeel (bool isLeftArrow) : isLeft (isLeftArrow) {}

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
    {
        // テキストは描画せず、代わりに三角形を描画するため、この関数は空にします。
        juce::ignoreUnused(g, button, isMouseOverButton, isButtonDown);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        // 1. 通常のボタン背景をベースクラスに描画してもらう（枠線や丸みなどを維持）
        juce::LookAndFeel_V4::drawButtonBackground (g, button, backgroundColour, isMouseOverButton, isButtonDown);

        // 2. 三角形（矢印グラフィック）の描画計算
        auto width = (float) button.getWidth();
        auto height = (float) button.getHeight();

        // ボタンの中心を基準に、綺麗な正三角形のサイズを決定
        auto arrowSize = juce::jmin (width, height) * 0.35f;
        auto centerX = width * 0.5f;
        auto centerY = height * 0.5f;

        juce::Path p;

        if (isLeft)
        {
            // 左向きの三角形 (◀)
            p.addTriangle (centerX - arrowSize * 0.5f, centerY,                  // 先端（左）
                           centerX + arrowSize * 0.5f, centerY - arrowSize * 0.4f, // 右上
                           centerX + arrowSize * 0.5f, centerY + arrowSize * 0.4f); // 右下
        }
        else
        {
            // 右向きの三角形 (▶)
            p.addTriangle (centerX + arrowSize * 0.5f, centerY,                  // 先端（右）
                           centerX - arrowSize * 0.5f, centerY - arrowSize * 0.4f, // 左上
                           centerX - arrowSize * 0.5f, centerY + arrowSize * 0.4f); // 左下
        }

        // 3. 三角形の色を決定（文字色と同じ設定を流用するか、固定の色にする）
        // ここではテキスト用カラー（デフォルトなら白や薄いグレー）を取得して塗ります
        auto arrowColor = button.findColour (juce::TextButton::textColourOffId);

        // マウスホバーやクリック時に少し色を変化させる（お好みで調整してください）
        if (isButtonDown)      g.setColour (arrowColor.withAlpha (0.6f));
        else if (isMouseOverButton) g.setColour (arrowColor.brighter (0.1f));
        else                   g.setColour (arrowColor);

        g.fillPath (p);
    }

private:
    bool isLeft;
};

// ==============================================================================
// メインコンポーネントクラス
// ==============================================================================
class MC500_InputComponent  : public juce::Component,
                              private juce::Timer
{
public:
    MC500_InputComponent();
    ~MC500_InputComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void visibilityChanged() override;

private:
    void timerCallback() override;

    // ダイヤルが現在どのパラメーターを操作しているかを表す状態定義
    enum class DialTargetMode
    {
        None,
        Measure,
        Beat,
        Clock,
        StepTime,
        Note,
        Velocity,
        GateTime
    };

    DialTargetMode currentDialMode = DialTargetMode::StepTime; // 初期状態はStepTime

    // --- UI素材の定義 ---
    LcdComponent lcdArea;

    InfiniteRotarySliderComponent alphaDialSlider;
    float dialVisualAngle = 0.0f;
    juce::TextButton leftButton;
    juce::TextButton rightButton;

    // 中央のボタン配列 (OwnedArrayはスマートポインタの動的配列)
    juce::OwnedArray<juce::TextButton> centerButtons;

    // 右のテンキー配列
    juce::OwnedArray<juce::TextButton> numButtons;
    juce::TextButton enterButton;

    // ★カスタムLook&Feelのインスタンスを追加
    DialLookAndFeel dialLookAndFeel;
    ArrowButtonLookAndFeel leftArrowLookAndFeel { true };
    ArrowButtonLookAndFeel rightArrowLookAndFeel { false };

    void setupButton (juce::TextButton& btn, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MC500_InputComponent)
};
