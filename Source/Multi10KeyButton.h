#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

class Multi10KeyButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class Multi10KeyDrawMode
    {
        Number,
        NoteName,
        NoteDuration
    };

    Multi10KeyButtonLookAndFeel (int number) : num (number), drawMode(Multi10KeyDrawMode::Number) {}

    void setDrawMode(Multi10KeyDrawMode mode)
    {
        drawMode = mode;
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
    {
        auto textColor = button.findColour (juce::TextButton::textColourOffId);
        if (isButtonDown)           g.setColour (textColor.withAlpha (0.6f));
        else if (isMouseOverButton) g.setColour (textColor.brighter (0.1f));
        else                        g.setColour (textColor);

        // フォントの共通設定
        auto bounds = button.getLocalBounds().toFloat();
        float fontSize = bounds.getHeight() * 0.36f;
        g.setFont (juce::Font (fontSize, juce::Font::plain));

        switch (drawMode)
        {
            case Multi10KeyDrawMode::Number:
            {
                auto buttonText = juce::String(num);
                g.drawText (buttonText, bounds, juce::Justification::centred, true);
                break;
            }
            case Multi10KeyDrawMode::NoteName:
            {
                if (num >= 0 && num < 10)
                {
                    auto buttonText = noteName[static_cast<size_t>(num)];
                    g.drawText (buttonText, bounds, juce::Justification::centred, true);
                }
                break;
            }
            case Multi10KeyDrawMode::NoteDuration:
            {
                // ボタンのサイズより少し小さめ（25%の余白）の描画エリアを設定
                auto targetArea = bounds.reduced (bounds.getWidth() * 0.30f,
                                                 bounds.getHeight() * 0.30f);
                juce::Path notePath;
                switch(num)
                {
                    case 0:
                    {
                        break;
                    }
                    case 1:
                    {
                        notePath = create64SplitNotePath();
                        break;
                    }
                    case 2:
                    {
                        notePath = create32SplitNotePath();
                        break;
                    }
                    case 3:
                    {
                        notePath = create16SplitNotePath();
                        juce::Path threePath = createTripletThreePath();
                        notePath.addPath (threePath);
                        break;
                    }
                    case 4:
                    {
                        notePath = create16SplitNotePath();
                        break;
                    }
                    case 5:
                    {
                        notePath = create8SplitNotePath();
                        juce::Path threePath = createTripletThreePath();
                        notePath.addPath (threePath);
                        break;
                    }
                    case 6:
                    {
                        notePath = create8SplitNotePath();
                        break;
                    }
                    case 7:
                    {
                        notePath = create4SplitNotePath();
                        juce::Path threePath = createTripletThreePath();
                        notePath.addPath (threePath);
                        break;
                    }
                    case 8:
                    {
                        notePath = create4SplitNotePath();
                        break;
                    }
                    case 9:
                    {
                        notePath = create2SplitNotePath();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }


                // アスペクト比を維持したまま、targetAreaの中央（centred）に配置するアフィン変換を取得
                auto transform = notePath.getTransformToScaleToFit (targetArea,
                                                                    true,
                                                                    juce::Justification::centred);

                g.fillPath (notePath, transform);
                break;
            }
        }
    }

private:
    // 💡 inline static を使うことで、クラス内での宣言と初期化が同時に完結します
    inline static const std::array<juce::String, 10> noteName
    {
        "A", "B", "C", "D", "E",
        "F", "G", "b", "#", ""
    };

    int num;
    Multi10KeyDrawMode drawMode;

    // 共通で使うノートヘッド（玉）とステム（棒）を構築するヘルパー関数
    void buildBaseNote (juce::Path& p, bool isFilled)
    {
        p.clear();

        // 1. ノートヘッド（玉）: 15度ほど右上がりに傾いた楕円
        // MC-50の印字は少し潰れたシャープな楕円です
        juce::Path head;
        head.addEllipse (0, 0, 34, 22);
        head.applyTransform (juce::AffineTransform::rotation (-0.26f, 17.0f, 11.0f)
                                                      .translated (20.0f, 65.0f));

        if (isFilled)
        {
            p.addPath (head);
        }
        else
        {
            // 二分音符用に中抜きの輪郭線にする（線幅3.5ピクセル）
            juce::PathStrokeType (3.5f).createStrokedPath (p, head);
        }

        // 2. ステム（棒）: 玉の右端から上に真っ直ぐ伸びる直線（長方形）
        p.addRectangle (50.5f, 15.0f, 3.5f, 60.0f);
    }

    // --- 各音符のパスを生成する関数群 ---

    // 二分音符: 白抜き玉 + 棒
    juce::Path create2SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, false); // false = 白抜き
        return p;
    }

    // 四分音符: 白抜き玉 + 棒
    juce::Path create4SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, true); // true = 黒塗り
        return p;
    }

    // 八分音符: 黒塗り玉 + 棒 + 1本の旗
    juce::Path create8SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, true); // true = 黒塗り

        // 旗（フラグ）: 棒の右上(54, 15)から右下に降りるシャープなポリゴン
        p.startNewSubPath (54.0f, 15.0f);
        p.lineTo (72.0f, 25.0f);
        p.lineTo (72.0f, 40.0f);
        p.lineTo (54.0f, 30.0f);
        p.closeSubPath();

        return p;
    }

    // 十六分音符: 黒塗り玉 + 棒 + 2本の旗
    juce::Path create16SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, true);

        // 1本目の旗
        p.startNewSubPath (54.0f, 15.0f);
        p.lineTo (72.0f, 25.0f);
        p.lineTo (72.0f, 37.0f);
        p.lineTo (54.0f, 27.0f);
        p.closeSubPath();

        // 2本目の旗（少し下げて平行に配置）
        p.startNewSubPath (54.0f, 32.0f);
        p.lineTo (72.0f, 42.0f);
        p.lineTo (72.0f, 54.0f);
        p.lineTo (54.0f, 44.0f);
        p.closeSubPath();

        return p;
    }

    // 三十二分音符: 黒塗り玉 + 棒 + 3本の旗
    juce::Path create32SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, true);

        // 3本の旗を等間隔でタイトに並べる（MC-50は非常に詰まったソリッドな見た目です）
        for (int i = 0; i < 3; ++i)
        {
            float yOffset = i * 13.0f;
            p.startNewSubPath (54.0f, 15.0f + yOffset);
            p.lineTo (72.0f, 23.0f + yOffset);
            p.lineTo (72.0f, 32.0f + yOffset);
            p.lineTo (54.0f, 24.0f + yOffset);
            p.closeSubPath();
        }

        return p;
    }

    // 六十四分音符: 黒塗り玉 + 棒 + 4本の旗
    juce::Path create64SplitNotePath()
    {
        juce::Path p;
        buildBaseNote (p, true);

        // 4本の旗（さらに細かく配置。ステムの下部まで旗が届く独特のメカニカルさ）
        for (int i = 0; i < 4; ++i)
        {
            float yOffset = i * 11.0f;
            p.startNewSubPath (54.0f, 13.0f + yOffset);
            p.lineTo (72.0f, 20.0f + yOffset);
            p.lineTo (72.0f, 27.0f + yOffset);
            p.lineTo (54.0f, 20.0f + yOffset);
            p.closeSubPath();
        }

        return p;
    }

    // 7セグメントディスプレイやカッティングシートのような直線的な「3」を構築
    juce::Path createTripletThreePath()
    {
        juce::Path p;

        // 配置エリア: X=70~88, Y=35~65 付近（音符の横）
        float yOffset = 20.0f;
        p.startNewSubPath (70.0f, 35.0f + yOffset);
        p.lineTo (88.0f, 35.0f + yOffset);
        p.lineTo (88.0f, 65.0f + yOffset);
        p.lineTo (70.0f, 65.0f + yOffset);
        p.lineTo (70.0f, 59.0f + yOffset);
        p.lineTo (83.0f, 59.0f + yOffset);
        p.lineTo (83.0f, 52.0f + yOffset);
        p.lineTo (73.0f, 52.0f + yOffset);
        p.lineTo (73.0f, 47.0f + yOffset);
        p.lineTo (83.0f, 47.0f + yOffset);
        p.lineTo (83.0f, 41.0f + yOffset);
        p.lineTo (70.0f, 41.0f + yOffset);
        p.closeSubPath();

        return p;
    }
};


class Multi10KeyButton  : public juce::TextButton
{
public:
    /**
     * コンストラクター
     * @param number ボタンに対応する数値 (0-9)
     */
    Multi10KeyButton (int number);

    /** デストラクター */
    ~Multi10KeyButton() override;

    void setDrawMode(Multi10KeyButtonLookAndFeel::Multi10KeyDrawMode newMode);

private:
    // このボタン専用の LookAndFeel をスマートポインタで安全に保持
    std::unique_ptr<Multi10KeyButtonLookAndFeel> buttonLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Multi10KeyButton)
};
