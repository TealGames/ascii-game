#pragma once
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include "TextBuffer.hpp"

using RenderLayerNumericType = std::uint8_t;
enum class RenderLayerType : RenderLayerNumericType
{
    None= 0,
    Background= 1<<0,
    Player= 1 << 1,
    UI= 1 << 2,
};
std::string ToString(const RenderLayerType& layer);
std::vector<std::string> GetLayersAsStrings(const RenderLayerType& layers);
RenderLayerType GetLayersFromStrings(const std::vector<std::string> strings);

RenderLayerType operator|(const RenderLayerType& lhs, const RenderLayerType& rhs);
RenderLayerType& operator|=(RenderLayerType& lhs, const RenderLayerType& rhs);
RenderLayerType operator&(const RenderLayerType& lhs, const RenderLayerType& rhs);
RenderLayerType& operator&=(RenderLayerType& lhs, const RenderLayerType& rhs);

bool operator==(const RenderLayerType& lhs, const RenderLayerNumericType& rhs);
bool operator!=(const RenderLayerType& lhs, const RenderLayerNumericType& rhs);

//using RawTextBufferBlock = std::vector<std::vector<TextCharPosition>>;
class RenderLayer
{
private:
   /* const int m_fontSize;
   
    RawTextBufferBlock m_rawTextBuffer;
    TextBuffer m_defaultSquaredTextBuffer;*/
    FragmentedTextBuffer m_buffer;

public:
    //const Utils::Point2DInt m_CharSpacing;

    //TODO: exposing the whole buffer to be mutated is unsafe there should be a better way to mutate it like maybe with a function
    //and should be done through the layer so it could valdiate any requests for changing the buffer (when there is not much
    //posibility for mistake it is fine, but when complexity increases it might become more necccessary)
    //The text buffer here is organized into a rectangle unlike the raw version
   /* TextBuffer m_SquaredTextBuffer;*/

    //Where the text buffer starts relative to the position of the max text buffer
    //Utils::Point2DInt m_StartPos;
   

private:
    //std::optional<TextBuffer> CreateSquaredBuffer() const;

public:
    RenderLayer();
    RenderLayer(const FragmentedTextBuffer& buffer);

    void AddText(const TextBufferCharPosition& bufferPos);

    FragmentedTextBuffer& GetBufferMutable();
    const FragmentedTextBuffer& GetBuffer() const;
    /*/// <summary>
    /// If the buffer is not fully square, you can choose this constructor to 
    /// shape it into a square shape
    /// </summary>
    /// <param name="rawBuffer"></param>
    /// <param name="fontSize"></param>
    /// <param name="charSpacing"></param>
    RenderLayer(const RawTextBufferBlock& rawBuffer, const int& fontSize, 
        const Utils::Point2DInt& charSpacing);

    /// <summary>
    /// This constructor requires a perfect square buffer
    /// </summary>
    /// <param name="squareBuffer"></param>
    /// <param name="fontSize"></param>
    /// <param name="charSpacing"></param>
    RenderLayer(const TextBuffer& squareBuffer, const int& fontSize,
        const Utils::Point2DInt& charSpacing);*/

    /// <summary>
    /// Resets the buffer back to the original
    /// </summary>
    void ResetToDefault();

    /*/// <summary>
    /// Will return the amount of spacing between characters in (WIDTH, HEIGHT)
    /// </summary>
    /// <returns></returns>
    Utils::Point2DInt CalculateCharSpacing() const;*/

    //static std::string ToStringRawBuffer(const RawTextBufferBlock& block);
    std::string ToString() const;
};

