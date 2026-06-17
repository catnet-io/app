export namespace engine {
	
	export class ScanConfig {
	    defaultPorts: number[];
	    portTimeoutMs: number;
	    pingTimeoutMs: number;
	    maxThreads: number;
	
	    static createFrom(source: any = {}) {
	        return new ScanConfig(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.defaultPorts = source["defaultPorts"];
	        this.portTimeoutMs = source["portTimeoutMs"];
	        this.pingTimeoutMs = source["pingTimeoutMs"];
	        this.maxThreads = source["maxThreads"];
	    }
	}

}

export namespace results {
	
	export class DeviceInfo {
	    ip: string;
	    isAlive: boolean;
	    hostname: string;
	    mac: string;
	    os?: string;
	    osFamily?: string;
	    deviceType?: string;
	    vendor?: string;
	    openPorts: number[];
	
	    static createFrom(source: any = {}) {
	        return new DeviceInfo(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.isAlive = source["isAlive"];
	        this.hostname = source["hostname"];
	        this.mac = source["mac"];
	        this.os = source["os"];
	        this.osFamily = source["osFamily"];
	        this.deviceType = source["deviceType"];
	        this.vendor = source["vendor"];
	        this.openPorts = source["openPorts"];
	    }
	}

}

