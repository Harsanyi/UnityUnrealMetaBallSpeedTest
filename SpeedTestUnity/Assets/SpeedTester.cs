using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;
using Unity.VisualScripting;
using System.Runtime.InteropServices;
using System;
using System.Threading.Tasks;
using System.IO;
using AOT;

public class SpeedTester : MonoBehaviour
{
    const String SETTINGS_PATH = "Settings.ini";

    [Header("Objs")]
    [SerializeField] ScriptMachine scriptMachine = null;
    [SerializeField] RawImage background = null;
    [SerializeField] ComputeShader metaBallShader = null;

    [Header("Settings")]
    [SerializeField, Range(20,120)] int targetFPS = 60;

    [Space]
    public int dotCount = 30;
    public int minDotRadious = 2;
    public int maxDotRadious = 30;
    public float border = 1f;
    public int runCount = 1000;

    [Space]
    public Color darkColor = Color.black;
    public Color lightColor = Color.white;

    [Space, Header("Events")]
    public UnityEvent<string> onLog;
    public UnityEvent onTestStart;
    public UnityEvent onTestEnded;

    public Texture2D texture;
    public MetaBall[] metaBalls;

    public bool singleThread = false;
    public bool bolt = false;
    public bool setPixelData = false;
    public bool cppNativePlugin = false;
    public bool multiThread = false;
    public bool cppMultiThread = false;
    public bool computeShader = false;

    public Vector2Int resolution { get; private set; } = new Vector2Int(320, 320);
    Coroutine routine;

    [DllImport("MetaBallSampler", EntryPoint = "getColorData")]
    private static extern System.IntPtr getColorData(
        System.IntPtr[] balls,
        int ballCount,
        float border,
        byte[] lightColor,
        byte[] darkColor,
        int width,
        int height,
        out int size
        );
    
    [DllImport("MetaBallSampler", EntryPoint = "getColorDataMultiThreaded")]
    private static extern System.IntPtr getColorDataMultiThreaded(
        System.IntPtr[] balls,
        int ballCount,
        float border,
        byte[] lightColor,
        byte[] darkColor,
        int width,
        int height,
        out int size
        );
    
    [DllImport("MetaBallSampler", EntryPoint = "freeMemory")]
    private static extern void freeMemory(System.IntPtr data);
    
    [DllImport("MetaBallSampler", EntryPoint = "createBall")]
    private static extern System.IntPtr createBall(int x, int y, float r);

    public delegate void LogCallback(string message);
    [MonoPInvokeCallback(typeof(LogCallback))]
    static void DebugLog(string message) {
        Debug.Log(message);
    }
    [DllImport("MetaBallSampler", EntryPoint = "registerLogCallback")]
    private static extern void registerLogCallback(LogCallback logCallback);

    void SaveSettingsAsync() {
        Task.Run(() => {
            using (FileStream saveStream = File.Create(SETTINGS_PATH)) {
                using (StreamWriter saveWriter = new StreamWriter(saveStream)) {
					saveWriter.WriteLine($"width={resolution.x}");
					saveWriter.WriteLine($"height={resolution.y}");
					saveWriter.WriteLine($"dotCount={dotCount}");
					saveWriter.WriteLine($"minDotRadious={minDotRadious}");
					saveWriter.WriteLine($"maxDotRadious={maxDotRadious}");
					saveWriter.WriteLine($"border={border}");
					saveWriter.WriteLine($"runCount={runCount}");
					saveWriter.WriteLine($"lightColor=#{UnityEngine.ColorUtility.ToHtmlStringRGBA(lightColor)}");
					saveWriter.WriteLine($"darkColor=#{UnityEngine.ColorUtility.ToHtmlStringRGBA(darkColor)}");
					saveWriter.WriteLine($"bolt={bolt}");
					saveWriter.WriteLine($"singleThread={singleThread}");
					saveWriter.WriteLine($"multiThread={multiThread}");
					saveWriter.WriteLine($"setPixelData={setPixelData}");
					saveWriter.WriteLine($"cppNativePlugin={cppNativePlugin}");
					saveWriter.WriteLine($"cppMultiThread={cppMultiThread}");
					saveWriter.WriteLine($"computeShader={computeShader}");
				}
            }
        });
    }

    bool TryLoadSettings() {
        if (File.Exists(SETTINGS_PATH))
        {
            List<(string key, string value)> values = new List<(string key, string value)>();

            using (FileStream stream = File.OpenRead(SETTINGS_PATH)) {
                using (StreamReader reader = new StreamReader(stream)) {
                    string line;
                    while(!String.IsNullOrEmpty(line = reader.ReadLine())){
                        values.Add((line.PartBefore('='), line.PartAfter('=')));
                    }
                }
            }

            string GetValueFor(string key) {
                for (int i = 0; i < values.Count; i++) {
                    if (values[i].key == key) {
                        return values[i].value;
                    }
                }

                throw new SystemException($"No value found for key \"{key}\"");
            }

            try
            {
                resolution = new Vector2Int(int.Parse(GetValueFor("width")), int.Parse(GetValueFor("height")));
				dotCount = int.Parse(GetValueFor("dotCount"));
				minDotRadious = int.Parse(GetValueFor("minDotRadious"));
				maxDotRadious = int.Parse(GetValueFor("maxDotRadious"));
				border = float.Parse(GetValueFor("border"));
                runCount = int.Parse(GetValueFor("runCount"));

                if (!UnityEngine.ColorUtility.TryParseHtmlString(GetValueFor("lightColor"), out lightColor) ||
                    !UnityEngine.ColorUtility.TryParseHtmlString(GetValueFor("darkColor"), out darkColor)) {
                    throw new System.Exception("Wrong color format!");
                }

                bolt = bool.Parse(GetValueFor("bolt"));
                singleThread = bool.Parse(GetValueFor("singleThread"));
                multiThread = bool.Parse(GetValueFor("multiThread"));
                cppNativePlugin = bool.Parse(GetValueFor("cppNativePlugin"));
                cppMultiThread = bool.Parse(GetValueFor("cppMultiThread"));
                setPixelData = bool.Parse(GetValueFor("setPixelData"));
                computeShader = bool.Parse(GetValueFor("computeShader"));
			}
            catch (System.Exception e) {
                Debug.LogException(e);
            }

            return true;
        }
        else {
            return false;
        }
    }

    private void Awake()
    {
        Application.targetFrameRate = targetFPS;

        registerLogCallback(DebugLog);

        if (!TryLoadSettings())
        {
			SaveSettingsAsync();
        }
    }

    private void OnDestroy()
    {
        Destroy(texture);
    }

    public void SetResolution(int width, int height) {
        resolution = new Vector2Int(width,height);
    }

    public void StartTest() {
        if (routine != null) StopCoroutine(routine);
        routine = StartCoroutine(TestRoutine());
    }

    IEnumerator TestRoutine() {
        try {
            onTestStart?.Invoke();
        }catch (System.Exception e){
            Debug.LogException(e);
        }

        //Generate base texture
        background.enabled = true;
        if (texture) Destroy(texture);
        texture = new Texture2D(resolution.x, resolution.y, TextureFormat.RGB24, false);
        background.texture = texture;

        //Generate Dots
        GenerateMetaBalls();

        Log("------[Start Testing]------");
        yield return null;

        //Single Thread
        if (singleThread) {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                SingleThreadTextureUpdate();
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Log($"Single thread:{avgTime}");
        }

        //Bolt
        if (bolt) {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                scriptMachine.TriggerUnityEvent("UpdateTexture");
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Log($"Bolt:{avgTime}");
        }

        //Byte[]
        if (setPixelData) {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                SetPixelDataUpdate();
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Debug.Log(watch.ElapsedMilliseconds);
            Log($"SetPixelData:{avgTime}");
        }

        //Cpp native
        if (cppNativePlugin) {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                CppNativePluginUpdate(false);
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Debug.Log(watch.ElapsedMilliseconds);
            Log($"Cpp Single Thread:{avgTime}");
        }

        //Multithread
        if (multiThread)
        {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                MultyThreadTextureUpdate();
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Debug.Log(watch.ElapsedMilliseconds);
            Log($"MultiThreaded:{avgTime}");
        }

        //cppMultiThread
        if (cppMultiThread) {
            yield return null;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            watch.Start();

            for (int i = 0; i < runCount; ++i)
            {
                CppNativePluginUpdate(true);
            }

            watch.Stop();
            long avgTime = watch.ElapsedMilliseconds / runCount;
            Log($"Cpp Multithreaded:{avgTime}");
        }

        //ComputeShader
        if (computeShader) {
			yield return null;

			System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
			watch.Start();

			for (int i = 0; i < runCount; ++i)
			{
                ComputeShaderUpdate();
			}

			watch.Stop();
			long avgTime = watch.ElapsedMilliseconds / runCount;
			Log($"ComputeShader:{avgTime}");
		}

        Log("------[Testing Finished]------");

        try
        {
            onTestEnded?.Invoke();
        }
        catch (System.Exception e)
        {
            Debug.LogException(e);
        }
    }

    void Log(string text) {
        Debug.Log(text);
        try
        {
            onLog?.Invoke(text);
        }
        catch (System.Exception e) {
            Debug.LogException(e);
        }
    }

    void GenerateMetaBalls() {
        metaBalls = new MetaBall[dotCount];
        for (int i = 0; i < dotCount; ++i) {
            metaBalls[i] = new MetaBall();
            metaBalls[i].x = UnityEngine.Random.Range(0,texture.width);
            metaBalls[i].y = UnityEngine.Random.Range(0,texture.height);
            metaBalls[i].r = UnityEngine.Random.Range(minDotRadious,maxDotRadious+1);
        }
    }

    void CppNativePluginUpdate(bool multithreaded) {
        System.IntPtr[] balls = new IntPtr[metaBalls.Length];
        for (int i = 0; i < metaBalls.Length; i++)
        {
            balls[i] = createBall(metaBalls[i].x, metaBalls[i].y, metaBalls[i].r);
        }
        byte[] lCol = new byte[] { (byte)(lightColor.r * 255), (byte)(lightColor.g * 255), (byte)(lightColor.b * 255) };
        byte[] dCol = new byte[] { (byte)(darkColor.r * 255), (byte)(darkColor.g * 255), (byte)(darkColor.b * 255) };

        int size;
        System.IntPtr data;
        if (multithreaded)
        {
            data = getColorDataMultiThreaded(balls, balls.Length, border, lCol, dCol, resolution.x, resolution.y, out size);
        }
        else {
            data = getColorData(balls, balls.Length, border, lCol, dCol, resolution.x, resolution.y, out size);
        }
        
        byte[] colorData = new byte[size];
        Marshal.Copy(data, colorData, 0, size);
        freeMemory(data);
        for (int i = 0; i < balls.Length; i++)
        {
            freeMemory(balls[i]);
        }

        texture.SetPixelData(colorData, 0);
        texture.Apply();
    }

    void SetPixelDataUpdate() {
        byte[] data = new byte[texture.width * texture.height * 3];

        int i;
        Color col;
        for (int x = 0; x < texture.width; x++)
        {
            for (int y = 0; y < texture.height; y++)
            {
                i = (x * texture.width + y) * 3;
                col = GetSample(ref x, ref y) >= border ? lightColor : darkColor;

                data[i] = (byte)(col.r * 255);
                data[i + 1] = (byte)(col.g * 255);
                data[i + 2] = (byte)(col.b * 255);
            }
        }

        texture.SetPixelData(data, 0);
        texture.Apply();
    }

    void SingleThreadTextureUpdate()
    {
        for (int x = 0; x < texture.width; ++x)
        {
            for (int y = 0; y < texture.height; ++y)
            {
                texture.SetPixel(x, y, GetSample(ref x, ref y) >= border ? lightColor : darkColor);
            }
        }
        texture.Apply();
    }

    void MultyThreadTextureUpdate()
    {
        Task[] tasks = new Task[System.Environment.ProcessorCount];
        byte[] data = new byte[texture.width * texture.height * 3];
        int tWidth = texture.width;
        int tHeight = texture.height;
        int xSize = texture.width / tasks.Length + 1;

        for (int t = 0; t < tasks.Length; t++) {
            int i;
            Color col;
            int Tc = t;

            tasks[t] = Task.Run(()=> {
                for (int x = Tc * xSize; x < tWidth && x<(Tc + 1)*xSize; x++)
                {
                    for (int y = 0; y < tHeight; y++)
                    {
                        i = (x * tWidth + y) * 3;
                        col = GetSample(ref x, ref y) >= border ? lightColor : darkColor;

                        data[i] = (byte)(col.r * 255);
                        data[i + 1] = (byte)(col.g * 255);
                        data[i + 2] = (byte)(col.b * 255);
                    }
                }
            });
        }

        Task.WaitAll(tasks);
        texture.SetPixelData(data, 0);
        texture.Apply();
    }

    void ComputeShaderUpdate() {
        int kernelIndex = metaBallShader.FindKernel("MetaBallShader");

        //Settings
        metaBallShader.SetInt("Width", texture.width);
        metaBallShader.SetInt("Height", texture.height);
        metaBallShader.SetInt("BallCount", metaBalls.Length);
        metaBallShader.SetFloat("Border", border);

        ComputeBuffer lightColorBuffer = new ComputeBuffer(3, sizeof(int));
		lightColorBuffer.SetData(new int[]{
            (int)(lightColor.r*255),
            (int)(lightColor.g*255),
            (int)(lightColor.b*255),
        });
        metaBallShader.SetBuffer(kernelIndex, "lightColor", lightColorBuffer);

        ComputeBuffer darkColorBuffer = new ComputeBuffer(3, sizeof(int));
        darkColorBuffer.SetData(new int[] { 
            (int)(darkColor.r*255),
			(int)(darkColor.g*255),
			(int)(darkColor.b*255),
		});
        metaBallShader.SetBuffer(kernelIndex, "darkColor", darkColorBuffer);

        //BallData
        ComputeBuffer BallX = new ComputeBuffer(metaBalls.Length, sizeof(int));
        int[] BallXData = new int[metaBalls.Length];
        ComputeBuffer BallY = new ComputeBuffer(metaBalls.Length, sizeof(int));
		int[] BallYData = new int[metaBalls.Length];
		ComputeBuffer BallR = new ComputeBuffer(metaBalls.Length, sizeof(float));
		float[] BallRData = new float[metaBalls.Length];
		for (int i = 0; i < metaBalls.Length; i++) {
            BallXData[i] = metaBalls[i].x;
            BallYData[i] = metaBalls[i].y;
            BallRData[i] = metaBalls[i].r;
        }
        BallX.SetData(BallXData);
        BallY.SetData(BallYData);
        BallR.SetData(BallRData);

        metaBallShader.SetBuffer(kernelIndex, "BallX", BallX);
		metaBallShader.SetBuffer(kernelIndex, "BallY", BallY);
		metaBallShader.SetBuffer(kernelIndex, "BallR", BallR);

        //Output
        ComputeBuffer OutputBuffer = new ComputeBuffer(texture.width*texture.height*3, sizeof(int));
        metaBallShader.SetBuffer(kernelIndex, "Output", OutputBuffer);

        //Dispatch
        metaBallShader.Dispatch(kernelIndex, 1, 1, 1);

		//Get Result
		int[] result = new int[texture.width*texture.height*3];
        OutputBuffer.GetData(result);
        //Cast to byte
        byte[] byteResult = new byte[result.Length];
        for (int i = 0; i < result.Length; i++) {
            byteResult[i] = (byte)result[i];
        }

        //Release buffers
        OutputBuffer.Release();
        BallX.Release();
        BallY.Release();
        BallR.Release();
		lightColorBuffer.Release();
        darkColorBuffer.Release();

        texture.SetPixelData(byteResult,0);
        texture.Apply();
    }

    float GetSample(ref int x,ref int y) {
        float totalInfluence = 0f;

        for (int i=0; i<metaBalls.Length; ++i) {
            totalInfluence += metaBalls[i].GetInfluenceTo(ref x,ref y);
        }

        return totalInfluence;
    }

	private void OnApplicationQuit()
	{
		SaveSettingsAsync();
	}
}

[System.Serializable, Inspectable]
public class MetaBall
{
    public int x;
    public int y;
    public float r;

    public float GetInfluenceTo(ref int x,ref int y)
    {
        if (this.x == x && this.y == y)
        {
            return r;
        }
        else {
			return 1f / (Mathf.Pow((this.x - x) / r, 2f) + Mathf.Pow((this.y - y) / r, 2f));
		}
    }
}
