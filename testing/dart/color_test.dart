// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:litetest/litetest.dart';

class NotAColor extends Color {
  const NotAColor(super.value);
}

Matcher approxEquals(dynamic o) => (v) {
  Expect.approxEquals(o as num, v as num);
};

void main() {
  test('color accessors should work', () {
    const Color foo = Color(0x12345678);
    expect(foo.alpha, equals(0x12));
    expect(foo.red, equals(0x34));
    expect(foo.green, equals(0x56));
    expect(foo.blue, equals(0x78));
  });

  test('paint set to black', () {
    const Color c = Color(0x00000000);
    final Paint p = Paint();
    p.color = c;
    expect(c.toString(), equals('Color(0x00000000)'));
  });

  test('color created with out of bounds value', () {
    const Color c = Color(0x100 << 24);
    final Paint p = Paint();
    p.color = c;
  });

  test('color created with wildly out of bounds value', () {
    const Color c = Color(1 << 1000000);
    final Paint p = Paint();
    p.color = c;
  });

  test('two colors are only == if they have the same runtime type', () {
    expect(const Color(0x12345678), equals(const Color(0x12345678)));
    expect(const Color(0x12345678), equals(Color(0x12345678))); // ignore: prefer_const_constructors
    expect(const Color(0x12345678), notEquals(const Color(0x87654321)));
    expect(const Color(0x12345678), notEquals(const NotAColor(0x12345678)));
    expect(const NotAColor(0x12345678), notEquals(const Color(0x12345678)));
    expect(const NotAColor(0x12345678), equals(const NotAColor(0x12345678)));
  });

  test('Color.lerp', () {
    expect(
      Color.lerp(const Color(0x00000000), const Color(0xFFFFFFFF), 0.0),
      const Color(0x00000000),
    );
    expect(
      Color.lerp(const Color(0x00000000), const Color(0xFFFFFFFF), 0.5),
      const Color(0x7F7F7F7F),
    );
    expect(
      Color.lerp(const Color(0x00000000), const Color(0xFFFFFFFF), 1.0),
      const Color(0xFFFFFFFF),
    );
    expect(
      Color.lerp(const Color(0x00000000), const Color(0xFFFFFFFF), -0.1),
      const Color(0x00000000),
    );
    expect(
      Color.lerp(const Color(0x00000000), const Color(0xFFFFFFFF), 1.1),
      const Color(0xFFFFFFFF),
    );

    // Prevent regression: https://github.com/flutter/flutter/issues/67423
    expect(
      Color.lerp(const Color(0xFFFFFFFF), const Color(0xFFFFFFFF), 0.04),
      const Color(0xFFFFFFFF),
    );
  });

  test('Color.lerp different colorspaces', () {
    bool didThrow = false;
    try {
      Color.lerp(
          const Color.from(
              alpha: 1,
              red: 1,
              green: 0,
              blue: 0,
              colorSpace: ColorSpace.displayP3),
          const Color.from(
              alpha: 1, red: 1, green: 0, blue: 0),
          0.0);
    } catch (ex) {
      didThrow = true;
    }
    expect(didThrow, isTrue);
  });

  test('Color.lerp same colorspaces', () {
    expect(
        Color.lerp(
            const Color.from(
                alpha: 1,
                red: 0,
                green: 0,
                blue: 0,
                colorSpace: ColorSpace.displayP3),
            const Color.from(
                alpha: 1,
                red: 1,
                green: 0,
                blue: 0,
                colorSpace: ColorSpace.displayP3),
            0.2),
        equals(
          const Color.from(
              alpha: 1,
              red: 0.2,
              green: 0,
              blue: 0,
              colorSpace: ColorSpace.displayP3),
        ));
  });

  test('Color.alphaBlend', () {
    expect(
      Color.alphaBlend(const Color(0x00000000), const Color(0x00000000)),
      const Color(0x00000000),
    );
    expect(
      Color.alphaBlend(const Color(0x00000000), const Color(0xFFFFFFFF)),
      const Color(0xFFFFFFFF),
    );
    expect(
      Color.alphaBlend(const Color(0xFFFFFFFF), const Color(0x00000000)),
      const Color(0xFFFFFFFF),
    );
    expect(
      Color.alphaBlend(const Color(0xFFFFFFFF), const Color(0xFFFFFFFF)),
      const Color(0xFFFFFFFF),
    );
    expect(
      Color.alphaBlend(const Color(0x80FFFFFF), const Color(0xFF000000)),
      const Color(0xFF808080),
    );
    expect(
      Color.alphaBlend(const Color(0x80808080), const Color(0xFFFFFFFF)),
      const Color(0xFFBFBFBF),
    );
    expect(
      Color.alphaBlend(const Color(0x80808080), const Color(0xFF000000)),
      const Color(0xFF404040),
    );
    expect(
      Color.alphaBlend(const Color(0x01020304), const Color(0xFF000000)),
      const Color(0xFF000000),
    );
    expect(
      Color.alphaBlend(const Color(0x11223344), const Color(0xFF000000)),
      const Color(0xFF020304),
    );
    expect(
      Color.alphaBlend(const Color(0x11223344), const Color(0x80000000)),
      const Color(0x88040608),
    );
  });

  test('Color.alphaBlend keeps colorspace', () {
    expect(
        Color.alphaBlend(
            const Color.from(
                alpha: 0.5,
                red: 1,
                green: 1,
                blue: 1,
                colorSpace: ColorSpace.displayP3),
            const Color.from(
                alpha: 1,
                red: 0,
                green: 0,
                blue: 0,
                colorSpace: ColorSpace.displayP3)),
        const Color.from(
            alpha: 1,
            red: 0.5,
            green: 0.5,
            blue: 0.5,
            colorSpace: ColorSpace.displayP3));
  });

  test('compute gray luminance', () {
    // Each color component is at 20%.
    const Color lightGray = Color(0xFF333333);
    // Relative luminance's formula is just the linearized color value for gray.
    // ((0.2 + 0.055) / 1.055) ^ 2.4.
    expect(lightGray.computeLuminance(), equals(0.033104766570885055));
  });

  test('compute color luminance', () {
    const Color brightRed = Color(0xFFFF3B30);
    // 0.2126 * ((1.0 + 0.055) / 1.055) ^ 2.4 +
    // 0.7152 * ((0.23137254902 +0.055) / 1.055) ^ 2.4 +
    // 0.0722 * ((0.18823529411 + 0.055) / 1.055) ^ 2.4
    expect(brightRed.computeLuminance(), equals(0.24601329637099723));
  });

  test('from and accessors', () {
    const Color color = Color.from(alpha: 0.1, red: 0.2, green: 0.3, blue: 0.4);
    expect(color.a, equals(0.1));
    expect(color.r, equals(0.2));
    expect(color.g, equals(0.3));
    expect(color.b, equals(0.4));
    expect(color.colorSpace, equals(ColorSpace.sRGB));

    expect(color.alpha, equals(26));
    expect(color.red, equals(51));
    expect(color.green, equals(77));
    expect(color.blue, equals(102));

    expect(color.value, equals(0x1a334d66));
  });

  test('fromARGB and accessors', () {
    const Color color = Color.fromARGB(10, 20, 35, 47);
    expect(color.alpha, equals(10));
    expect(color.red, equals(20));
    expect(color.green, equals(35));
    expect(color.blue, equals(47));
  });

  test('constructor and accessors', () {
    const Color color = Color(0xffeeddcc);
    expect(color.alpha, equals(0xff));
    expect(color.red, equals(0xee));
    expect(color.green, equals(0xdd));
    expect(color.blue, equals(0xcc));
  });

  test('p3 to extended srgb', () {
    const Color p3 = Color.from(
        alpha: 1, red: 1, green: 0, blue: 0, colorSpace: ColorSpace.displayP3);
    final Color srgb = p3.change(colorSpace: ColorSpace.extendedSRGB);
    expect(srgb.a, equals(1.0));
    expect(srgb.r, approxEquals(1.0931));
    expect(srgb.g, approxEquals(-0.22684034705162098));
    expect(srgb.b, approxEquals(-0.15007957816123998));
    expect(srgb.colorSpace, equals(ColorSpace.extendedSRGB));
  });

  test('p3 to srgb', () {
    const Color p3 = Color.from(
        alpha: 1, red: 1, green: 0, blue: 0, colorSpace: ColorSpace.displayP3);
    final Color srgb = p3.change(colorSpace: ColorSpace.sRGB);
    expect(srgb.a, equals(1.0));
    expect(srgb.r, approxEquals(1));
    expect(srgb.g, approxEquals(0));
    expect(srgb.b, approxEquals(0));
    expect(srgb.colorSpace, equals(ColorSpace.sRGB));
  });

  test('extended srgb to p3', () {
    const Color srgb = Color.from(
        alpha: 1,
        red: 1.0931,
        green: -0.2268,
        blue: -0.1501,
        colorSpace: ColorSpace.extendedSRGB);
    final Color p3 = srgb.change(colorSpace: ColorSpace.displayP3);
    expect(p3.a, equals(1.0));
    expect(p3.r, approxEquals(1));
    expect(p3.g, approxEquals(0));
    expect(p3.b, approxEquals(0));
    expect(p3.colorSpace, equals(ColorSpace.displayP3));
  });

  test('extended srgb to p3 clamped', () {
    const Color srgb = Color.from(
        alpha: 1,
        red: 2,
        green: 0,
        blue: 0,
        colorSpace: ColorSpace.extendedSRGB);
    final Color p3 = srgb.change(colorSpace: ColorSpace.displayP3);
    expect(srgb.a, equals(1.0));
    expect(p3.r <= 1.0, isTrue);
    expect(p3.g <= 1.0, isTrue);
    expect(p3.b <= 1.0, isTrue);
    expect(p3.r >= 0.0, isTrue);
    expect(p3.g >= 0.0, isTrue);
    expect(p3.b >= 0.0, isTrue);
  });

  test('hash considers colorspace', () {
    const Color srgb = Color.from(
        alpha: 1, red: 1, green: 0, blue: 0);
    const Color p3 = Color.from(
        alpha: 1, red: 1, green: 0, blue: 0, colorSpace: ColorSpace.displayP3);
    expect(srgb.hashCode, notEquals(p3.hashCode));
  });
}
