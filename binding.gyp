{
  "targets": [
    {
      "target_name": "lattice_native",
      "sources": [ "src/addon.cpp" ],
      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "Optimization": 3,
              "InlineFunctionExpansion": 2,
              "EnableIntrinsicFunctions": "true",
              "FavorSizeOrSpeed": 1,
              "EnableEnhancedInstructionSet": 3,
              "FloatingPointModel": 2
            }
          }
        }],
        ["OS!='win'", {
          "cflags_cc": [ "-O3", "-mavx2", "-mfma", "-ffast-math" ]
        }]
      ]
    }
  ]
}