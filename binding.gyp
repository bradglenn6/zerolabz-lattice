{
  "targets": [
    {
      "target_name": "lattice_native",
      "sources": [ "src/addon.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "conditions": [
        ['OS=="win"', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": [ "/arch:AVX2", "/std:c++20" ]
            }
          }
        }],
        ['OS!="win"', {
          "cflags_cc": [ "-std=c++20", "-mavx2", "-mfma", "-O3" ]
        }]
      ]
    }
  ]
}