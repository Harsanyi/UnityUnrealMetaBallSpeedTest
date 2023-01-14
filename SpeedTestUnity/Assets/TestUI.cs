using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using UnityEngine.UI;

public class TestUI : MonoBehaviour
{
    public static TestUI instance { get; private set; } = null;

    [SerializeField] TextMeshProUGUI logText = null;
    [SerializeField] SpeedTester tester = null;

    [Space,Header("Menu Elements")]
    [SerializeField] Button startTestButton = null;
    [SerializeField] Button exitButton = null;

    [Space]
    [SerializeField] TMP_InputField widthInput = null;
    [SerializeField] TMP_InputField heightInput = null;
    [SerializeField] TMP_InputField ballCountInput = null;
    [SerializeField] TMP_InputField minBallRadiousInput = null;
    [SerializeField] TMP_InputField maxBallRadiousInput = null;
    [SerializeField] TMP_InputField runCountInput = null;

    [Space]
    [SerializeField] Toggle singleToggle = null;
    [SerializeField] Toggle boltToggle = null;
    [SerializeField] Toggle setPixelDataToggle = null;
    [SerializeField] Toggle cppNativePluginToggle = null;
    [SerializeField] Toggle multiThreadedToggle = null;
    [SerializeField] Toggle cppMultiThreadToggle = null;
    [SerializeField] Toggle computeShaderToggle = null;

    private void Awake()
    {
        instance = this;
        tester.onLog.AddListener(Log);
        tester.onTestStart.AddListener(()=>startTestButton.interactable = false);
        tester.onTestEnded.AddListener(()=>startTestButton.interactable = true);
    }

    private void Start()
    {
        ClearLog();

        //Set Start Values
        singleToggle.SetIsOnWithoutNotify(tester.singleThread);
        boltToggle.SetIsOnWithoutNotify(tester.bolt);
        setPixelDataToggle.SetIsOnWithoutNotify(tester.setPixelData);
        cppNativePluginToggle.SetIsOnWithoutNotify(tester.cppNativePlugin);
        multiThreadedToggle.SetIsOnWithoutNotify(tester.multiThread);
        cppMultiThreadToggle.SetIsOnWithoutNotify(tester.cppMultiThread);
        computeShaderToggle.SetIsOnWithoutNotify(tester.computeShader);

		widthInput.SetTextWithoutNotify(tester.resolution.x.ToString());
        heightInput.SetTextWithoutNotify(tester.resolution.y.ToString());
        ballCountInput.SetTextWithoutNotify(tester.dotCount.ToString());
        minBallRadiousInput.SetTextWithoutNotify(tester.minDotRadious.ToString());
        maxBallRadiousInput.SetTextWithoutNotify(tester.maxDotRadious.ToString());
        runCountInput.SetTextWithoutNotify(tester.runCount.ToString());

        //Subscriptions
        startTestButton.onClick.AddListener(()=>tester.StartTest());
        exitButton.onClick.AddListener(()=>Application.Quit());

        singleToggle.onValueChanged.AddListener((on)=> {
            tester.singleThread = on;
        });
        boltToggle.onValueChanged.AddListener((on)=> {
            tester.bolt = on;
        });
        setPixelDataToggle.onValueChanged.AddListener((on)=> {
            tester.setPixelData = on;
        });
        cppNativePluginToggle.onValueChanged.AddListener((on)=> {
            tester.cppNativePlugin = on;
        });
        multiThreadedToggle.onValueChanged.AddListener((on)=> {
            tester.multiThread = on;
        });
        cppMultiThreadToggle.onValueChanged.AddListener((on) => {
            tester.cppMultiThread = on;
        });
        computeShaderToggle.onValueChanged.AddListener((on) => {
            tester.computeShader = on;
        });

        widthInput.onValueChanged.AddListener((widthText)=> {
            int newWidth;
            if (int.TryParse(widthText, out newWidth) && newWidth >= 0)
            {
                tester.SetResolution(newWidth, tester.resolution.y);
            }
        });
        heightInput.onValueChanged.AddListener((heightText)=> {
            int heightValue;
            if (int.TryParse(heightText, out heightValue) && heightValue > 0) {
                tester.SetResolution(tester.resolution.x, heightValue);
            }
        });
        ballCountInput.onValueChanged.AddListener((ballCountText)=> {
            int ballCountValue;
            if (int.TryParse(ballCountText, out ballCountValue) && ballCountValue >= 0) {
                tester.dotCount = ballCountValue;
            }
        });
        minBallRadiousInput.onValueChanged.AddListener((minBallradiousText)=> {
            int minBallRadiousValue;
            if (int.TryParse(minBallradiousText, out minBallRadiousValue) && minBallRadiousValue > 0) {
                tester.minDotRadious = minBallRadiousValue;
            }
        });
        maxBallRadiousInput.onValueChanged.AddListener((maxBallRadiousText) => {
            int maxBallRadiousValue;
            if (int.TryParse(maxBallRadiousText, out maxBallRadiousValue) && maxBallRadiousValue > tester.minDotRadious) {
                tester.maxDotRadious = maxBallRadiousValue;
            }
        });
        runCountInput.onValueChanged.AddListener((runCountText)=> {
            int runCountValue;
            if (int.TryParse(runCountText, out runCountValue) && runCountValue > 0) {
                tester.runCount = runCountValue;
            }
        });
    }

    public void ClearLog() {
        logText.text = "";
    }

    public void Log(string text) {
        logText.text = $"{text}\n{logText.text}";
    }
}
